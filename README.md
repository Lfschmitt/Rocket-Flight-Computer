# Rocket Flight Computer — Meteora (Missão Gorilla)

Computador de voo embarcado para o foguete **Meteora**, desenvolvido para a missão **Gorilla** com alcance de 1 km de altitude. O sistema realiza leitura de sensores a 50 Hz, telemetria sem fio via LoRa e registro de dados em cartão SD durante todo o voo.

---

## Visão Geral

| Parâmetro | Valor |
|---|---|
| Microcontrolador | ESP32 (dual-core) |
| Frequência do loop principal | 50 Hz |
| Transmissão LoRa | ~3.8 pacotes/s @ 915 MHz |
| Registro de dados | CSV em cartão SD |
| Altitude alvo | ~1 km |

---

## Hardware

| Módulo | Componente | Interface |
|---|---|---|
| Microcontrolador | ESP32 (Heltec) | — |
| Barômetro | BMP280 | I2C (0x77) |
| IMU (acelerômetro + giroscópio) | MPU6500 | I2C (0x68) |
| GPS | u-blox NEO-6M | UART2 (9600 baud) |
| Rádio LoRa | SX1276 | SPI |
| Armazenamento | Cartão SD | SPI |

### Pinagem (config.h)

```
I2C          → SDA: 21 | SCL: 22
SD Card      → CS: (definido em config.h), barramento SPI compartilhado
LoRa SX1276  → CS/DIO0/RST: (definido em config.h), barramento SPI compartilhado
GPS          → UART2
```

---

## Arquitetura de Software

O firmware utiliza **Arduino + FreeRTOS** no ESP32 com modelo dual-core:

```
Core 1 — Loop Principal (50 Hz)
├── Leitura BMP280 (altitude, pressão, temperatura)
├── Leitura MPU6500 (aceleração, giração)
├── Leitura GPS (posição, velocidade, HDOP)
├── SDCard.log()     → atualiza buffer compartilhado
└── LoRa.update()    → transmite pacote a cada 260 ms

Core 0 — Task FreeRTOS (SDCard)
└── writeTask()      → grava dados do buffer no arquivo CSV
```

### Sincronização SPI

O cartão SD e o rádio LoRa compartilham o mesmo barramento SPI. O acesso é coordenado por um **mutex FreeRTOS** (`spiMutex`) criado no `setup()` e passado para ambos os módulos, evitando corrupção de dados.

```
spiMutex ──→ SDCard  (writeTask, Core 0)
         └──→ LoRa    (transmit, Core 1)
```

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
│   ├── BMP280.h / .cpp          # Barômetro — altitude, pressão, temperatura
│
├── mpu6500/
│   ├── MPU6500.h / .cpp         # IMU 6 eixos — aceleração e giroscópio
│
├── gps/
│   ├── GYGPS.h / .cpp           # GPS u-blox NEO-6M via TinyGPSPlus
│
├── sdcard/
│   ├── SDCard.h / .cpp          # Registro CSV em SD — task dedicada no Core 0
│
├── telemetry/
│   ├── LoRa.h / .cpp            # Telemetria SX1276 @ 915 MHz via RadioLib
│
└── processing/
    ├── DataPrint.h / .cpp       # Saída serial formatada (debug)
    ├── SystemMonitor.h / .cpp   # Monitoramento de status dos módulos
```

---

## Dados de Voo (FlightData)

Todos os módulos leem e escrevem na estrutura central `FlightData`:

| Campo | Descrição | Unidade |
|---|---|---|
| `timestamp` | Tempo desde o boot | ms |
| `altitude` | Altitude barométrica (relativa ao solo) | m |
| `pressure` | Pressão absoluta | Pa |
| `temperature` | Temperatura | °C |
| `accel.x/y/z` | Aceleração (valor bruto do sensor) | g |
| `gyro.x/y/z` | Velocidade angular (delta da referência) | °/s |
| `latitude / longitude` | Coordenadas geográficas | ° |
| `gpsAltitude` | Altitude GPS | m |
| `gpsSpeed` | Velocidade GPS | m/s |
| `gpsHDOP` | Precisão horizontal | — |
| `satellites` | Satélites visíveis | — |
| `gpsFix` | Status do fix GPS | bool |
| `verticalVelocity` | Velocidade vertical calculada | m/s |
| `verticalAccel` | Aceleração vertical calculada | m/s² |
| `systemStatus` | Bitmask de status dos módulos | — |

### Status dos Módulos (bitmask)

| Bit | Módulo |
|---|---|
| 0 | BMP280 |
| 1 | MPU6500 |
| 2 | GPS |
| 3 | SD Card |
| 4 | LoRa |

---

## Configuração LoRa

| Parâmetro | Valor |
|---|---|
| Frequência | 915.0 MHz |
| Spreading Factor | SF9 |
| Bandwidth | 500 kHz |
| Coding Rate | 4/5 |
| Potência TX | 17 dBm (~50 mW) |
| Tamanho do pacote | 17 bytes (timestamp, lat, lng, altitude, systemStatus) |
| Taxa de transmissão | ~3.8 pacotes/s (a cada 260 ms) |

---

## Calibração

Na inicialização, o sistema coleta **50 amostras** de referência para:
- **BMP280**: estabelece pressão de referência ao nível do solo
- **MPU6500**: calibra offset do giroscópio (o acelerômetro não passa por calibração de offset — os valores lidos são brutos)

O DLPF do MPU6500 é configurado em **10 Hz** para filtrar vibrações do motor.

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

---

## Módulos em Desenvolvimento

Os seguintes módulos estão estruturados mas ainda não implementados:

- **DataProcessor** — processamento de dados em voo (velocidade e aceleração vertical)
- **ApogeeDetector** — detecção do apogeu com base em dados barométricos e IMU
- **StateMachine** — máquina de estados do voo (pré-lançamento → motor ativo → planagem → apogeu → descida → pousado)

---

## Autores

Desenvolvido por **Volkswangen T30**
Equipe de foguetemodelismo — Missão Gorilla / Foguete Meteora
