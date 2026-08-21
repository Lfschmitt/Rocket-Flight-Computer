# Rocket Flight Computer — Meteora (Missão Gorilla)

Computador de voo embarcado para o foguete **Meteora**, desenvolvido para a missão **Gorilla** com alcance de 1 km de altitude. O sistema realiza leitura de sensores a 50 Hz, telemetria sem fio via LoRa e registro de dados em cartão SD durante todo o voo.

---

## Visão Geral

| Parâmetro | Valor |
|---|---|
| Microcontrolador | ESP32 (Heltec WiFi LoRa 32) |
| Frequência do loop principal | 50 Hz (período de 20 ms) |
| Transmissão LoRa | 5 pacotes/s @ 915 MHz |
| Registro de dados | `/flightData.txt` no cartão SD (valores separados por vírgula) |
| Altitude alvo | ~1 km |

---

## Hardware

| Módulo | Componente | Interface |
|---|---|---|
| Microcontrolador | ESP32 (Heltec) | — |
| Barômetro | BME280 | I2C (0x77) |
| IMU (acelerômetro + giroscópio) | MPU9250 | I2C (0x68) |
| GPS | u-blox NEO-6M | UART2 (9600 baud) |
| Rádio LoRa | SX1276 | SPI |
| Armazenamento | Cartão SD | SPI |

### Pinagem (config.h)

```
LED de status → 32
I2C           → SDA: 21 | SCL: 22
SPI (SD+LoRa) → SCK: 5 | MISO: 19 | MOSI: 27
SD Card       → CS: 13
LoRa SX1276   → CS: 18 | RST: 14 | DIO0: 26
GPS (UART2)   → RX do ESP32: 25 | TX do ESP32: 17
```

---

## Arquitetura de Software

O firmware utiliza **Arduino + FreeRTOS** no ESP32 com modelo dual-core:

```
Core 1 — Loop Principal (50 Hz / 20 ms)
├── BMP280.read()         → altitude, pressão, temperatura
├── MPU6500.read()        → aceleração e giroscópio
├── GYGPS.feed/read()     → posição, velocidade, HDOP, satélites
├── SDCard.log()          → copia FlightData para o buffer compartilhado
├── LoRa.update()         → transmite 1 pacote a cada 10 ciclos (200 ms)
└── SystemMonitor.update()→ consolida o bitmask de status

Core 0 — Task FreeRTOS (SDCard)
└── writeTask()           → grava o buffer no arquivo a cada ~15 ms
```

### Sincronização SPI

O cartão SD e o rádio LoRa compartilham o mesmo barramento SPI. O acesso é coordenado por um **mutex FreeRTOS** (`spiMutex`) criado no `setup()` e passado para ambos os módulos, evitando corrupção de dados. Cada lado tenta tomar o mutex com timeout de 5 ms e pula o ciclo caso não consiga.

```
spiMutex ──→ SDCard  (write(), Core 0)
         └──→ LoRa    (startTransmit(), Core 1)
```

O SDCard mantém ainda um segundo mutex interno (`_dataMutex`) que protege o buffer `lastData` entre o `log()` (Core 1) e o `write()` (Core 0).

---

## Estrutura do Projeto

```
rocket_flight_computer/
├── rocket_flight_computer.ino   # Ponto de entrada — setup() e loop()
├── config.h                     # Pinos, endereços e parâmetros de configuração
├── FlightData.h                 # Estrutura central de dados de voo
├── modules.cpp                  # Agregador de compilação (Arduino IDE)
│
├── bmp280/
│   └── BMP280.h / .cpp          # Barômetro — altitude, pressão, temperatura
│
├── mpu6500/
│   └── MPU6500.h / .cpp         # IMU 6 eixos — aceleração e giroscópio
│
├── gps/
│   └── GYGPS.h / .cpp           # GPS u-blox NEO-6M via TinyGPSPlus
│
├── sdcard/
│   └── SDCard.h / .cpp          # Registro em SD — task dedicada no Core 0
│
├── telemetry/
│   └── LoRa.h / .cpp            # Telemetria SX1276 @ 915 MHz via RadioLib
│
└── processing/
    ├── DataPrint.h / .cpp       # Saída serial formatada (debug)
    └── SystemMonitor.h / .cpp   # Bitmask de status dos módulos
```

---

## Dados de Voo (FlightData)

Todos os módulos leem e escrevem na estrutura central `FlightData`:

| Campo | Descrição | Unidade |
|---|---|---|
| `timestamp` | Tempo desde o boot | ms |
| `altitude` | Altitude barométrica (relativa ao ponto de lançamento) | m |
| `pressure` | Pressão absoluta (filtrada) | Pa |
| `temperature` | Temperatura | °C |
| `accel.x/y/z` | Aceleração (filtrada, sem offset) | g |
| `gyro.x/y/z` | Velocidade angular (offset da calibração descontado) | °/s |
| `latitude / longitude` | Coordenadas geográficas | ° |
| `gpsAltitude` | Altitude GPS (acima do nível do mar) | m |
| `gpsSpeed` | Velocidade GPS | m/s |
| `gpsHDOP` | Precisão horizontal (<2 excelente, >5 ruim) | — |
| `satellites` | Satélites visíveis | — |
| `gpsFix` | Status do fix GPS | bool |
| `systemStatus` | Bitmask de status dos módulos | — |

### Status dos Módulos (bitmask)

| Bit | Máscara | Módulo |
|---|---|---|
| 0 | `SYS_BMP` (0x01) | BMP280 / BME280 |
| 1 | `SYS_MPU` (0x02) | MPU6500 / MPU9250 |
| 2 | `SYS_GPS` (0x04) | GPS |
| 3 | `SYS_SD` (0x08) | Cartão SD |
| 4 | `SYS_LORA` (0x10) | LoRa |

O bitmask é reconstruído a cada ciclo do loop a partir do retorno de cada `read()` / `log()`, ou seja, reflete o estado **da leitura atual**, não apenas o resultado do `setup()`.

---

## Registro em Cartão SD

O arquivo `/flightData.txt` é aberto em modo `FILE_APPEND` e recebe uma linha por ciclo de escrita, com os campos separados por vírgula, nesta ordem:

```
timestamp, systemStatus, altitude, pressure, temperature,
accel.x, accel.y, accel.z, gyro.x, gyro.y, gyro.z,
latitude, longitude, gpsAltitude, satellites
```

A gravação roda em uma task FreeRTOS fixada no **Core 0** (`sdWriteTask`, stack 4096, prioridade 1), com `vTaskDelay(15)` entre escritas.

---

## Configuração LoRa

| Parâmetro | Valor |
|---|---|
| Frequência | 915.0 MHz (faixa ISM liberada no Brasil) |
| Spreading Factor | SF9 |
| Bandwidth | 500 kHz |
| Coding Rate | 4/5 |
| Sync word | 0x12 |
| Preâmbulo | 8 símbolos |
| Potência TX | 17 dBm (~50 mW) |
| Tamanho do pacote | 17 bytes |
| Tempo no ar | ~51 ms |
| Taxa de transmissão | 5 pacotes/s (a cada 200 ms / 10 ciclos) |

### Formato do pacote (17 bytes, little-endian)

| Offset | Tamanho | Campo |
|---|---|---|
| 0 | 4 | `timestamp` (uint32, ms desde o boot) |
| 4 | 4 | `latitude` (float, graus decimais) |
| 8 | 4 | `longitude` (float, graus decimais) |
| 12 | 4 | `gpsAltitude` (float, m acima do nível do mar) |
| 16 | 1 | `systemStatus` (bitmask) |

A transmissão é assíncrona: `startTransmit()` retorna imediatamente e o fim do envio é sinalizado por interrupção no pino DIO0. O `init()` tenta inicializar o rádio até **3 vezes** com 200 ms de intervalo, e `getLastError()` devolve o código RadioLib da última falha.

---

## Calibração e Filtragem

Na inicialização:

- **BME280**: média de **50 amostras** de pressão define a referência do nível do solo; a altitude sai da equação hipsométrica. Sampling: temperatura ×2, pressão ×16, umidade ×1, filtro IIR ×16, standby 0,5 ms.
- **MPU9250**: média de **50 amostras** calibra o offset do **giroscópio**. O acelerômetro **não** passa por calibração de offset — os valores são brutos. Escalas: ±16 g e ±500 °/s, FIFO a 125 Hz.

Em voo, todas as leituras passam por uma média móvel exponencial cujos pesos ficam em `config.h`:

| Constante | Valor | Aplicação |
|---|---|---|
| `BME280_SENSIBILITY` | 0.95 | pressão |
| `MPU_ACCEL_SENSIBILITY` | 0.95 | aceleração |
| `MPU_GYRO_SENSIBILITY` | 0.9 | giroscópio |

O DLPF do MPU9250 (acelerômetro e giroscópio) é configurado em **10 Hz** para filtrar vibrações do motor.

---

## Dependências (Arduino Libraries)

| Biblioteca | Uso |
|---|---|
| `Adafruit_BME280` | Driver do barômetro (sensor BME280, substituto de hardware do BMP280 — mesma lógica e pinagem, classe mantida como `BMP280`) |
| `MPU9250` | Driver do IMU (sensor MPU9250, substituto de hardware do MPU6500 — mesma lógica e pinagem, classe mantida como `MPU6500`) |
| `TinyGPSPlus` | Parser NMEA para GPS u-blox |
| `RadioLib` | Driver LoRa SX1276 |
| `SD` | Acesso ao cartão SD (built-in ESP32) |
| `Wire` | I2C (built-in) |
| `SPI` | SPI (built-in) |
| `freertos/semphr.h` | Mutexes FreeRTOS (built-in ESP32) |

### Instalação via Arduino IDE

1. Abrir **Ferramentas → Gerenciar Bibliotecas**
2. Instalar: `Adafruit BME280`, `MPU9250`, `TinyGPSPlus`, `RadioLib`
3. Selecionar a placa **ESP32 Dev Module** (ou Heltec WiFi LoRa 32)

---

## Como Compilar e Gravar

1. Instalar o [Arduino IDE](https://www.arduino.cc/en/software) com suporte ao ESP32
2. Adicionar as bibliotecas listadas acima
3. Abrir `rocket_flight_computer.ino`
4. Selecionar a placa correta e a porta COM
5. Compilar e gravar (`Ctrl+U`)

> **Nota**: O arquivo `modules.cpp` é um agregador necessário para que o Arduino IDE compile corretamente os módulos em subpastas. Não remova este arquivo.

### Debug

A impressão formatada no monitor serial (115200 baud) fica desligada por padrão. Para ativar, descomente a chamada de `dataPrint.printFlightData(flightData)` no `loop()` — em voo ela deve permanecer comentada, pois consome tempo do ciclo.

---

## Autores

Desenvolvido por **Volkswangen T30**
Equipe ITA Rocket Design — Missão Gorilla / Foguete Meteora
