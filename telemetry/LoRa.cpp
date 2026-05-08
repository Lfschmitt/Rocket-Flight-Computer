#include "LoRa.h"

// Reserva o espaço em memória e inicializa como false (nenhuma transmissão concluída ainda)
volatile bool LORAMODULE::_txDone = false;

// Callback da interrupção do pino DIO0
// Chamado automaticamente pelo hardware quando o rádio termina de transmitir
// Só sinaliza o flag
void IRAM_ATTR LORAMODULE::_onTxDone() {
    _txDone = true;
}

bool LORAMODULE::init(SemaphoreHandle_t spiMutex) {
    // Guarda o mutex recebido do .ino para usar dentro de update()
    _spiMutex = spiMutex;

    // Códigos de retorno possíveis do RadioLib em _radio.begin():
    //   0  RADIOLIB_ERR_NONE              — sucesso
    //  -2  RADIOLIB_ERR_CHIP_NOT_FOUND    — SPI não encontrou o SX1276 (mau contato, pino errado)
    //  -3  RADIOLIB_ERR_INVALID_FREQUENCY — frequência fora da faixa do chip
    //  -4  RADIOLIB_ERR_INVALID_BANDWIDTH — BW inválido (valores válidos: 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500 kHz)
    //  -5  RADIOLIB_ERR_INVALID_SPREADING_FACTOR — SF fora de [6..12]
    //  -6  RADIOLIB_ERR_INVALID_CODING_RATE       — CR fora de [5..8]
    //  -7  RADIOLIB_ERR_INVALID_OUTPUT_POWER      — potência fora do limite do chip
    // -10  RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH   — preâmbulo fora de [6..65535]

    const int MAX_TRIES = 3;
    for (int attempt = 1; attempt <= MAX_TRIES; attempt++) {
        // Configura o chip SX1276 via SPI com os parâmetros de RF
        int state;
        if(xSemaphoreTake(_spiMutex, pdMS_TO_TICKS(500))){
            state = _radio.begin(
                LORA_FREQ,  // 915.0 MHz — faixa ISM do Brasil (definido em config.h)
                500.0,      // bandwidth 500 kHz — reduz tempo no ar (~72ms para 44 bytes)
                9,          // spreading factor 9 — alcance ~4km com boa velocidade
                5,          // coding rate 4/5 — mínima redundância, SF já garante robustez
                0x12,       // sync word — "senha" da rede; receptor precisa do mesmo valor
                17,         // potência 17 dBm (~50mW) — dentro do limite legal ANATEL 915MHz
                8           // 8 símbolos de preâmbulo — padrão LoRa
            );
            xSemaphoreGive(_spiMutex);
        }else{
            state = -99;
        }
        _lastError = state;

        if (state == RADIOLIB_ERR_NONE) {
            if (attempt > 1) {
                Serial.print("[OK] LoRa inicializado na tentativa ");
                Serial.println(attempt);
            }
            // Registra o callback no pino DIO0
            // RISING = dispara quando DIO0 sobe de LOW para HIGH, sinal que o SX1276 emite ao terminar TX
            _radio.setDio0Action(_onTxDone, RISING);
            return true;
        }

        Serial.print("[WARN] LoRa tentativa ");
        Serial.print(attempt);
        Serial.print("/");
        Serial.print(MAX_TRIES);
        Serial.print(" falhou — state=");
        Serial.println(state);

        if (attempt < MAX_TRIES) delay(200);
    }
    return false;
}

void LORAMODULE::update(const FlightData& data) {
    // ISR sinalizou que o envio anterior terminou — limpa os flags para liberar o próximo envio
    if (_txBusy && _txDone) {
        _txBusy = false;
        _txDone = false;
    }

    // Transmite 1 vez a cada 13 ciclos (130ms a 100Hz = ~7 pacotes por segundo)
    // Na maioria dos ciclos o update() termina aqui em menos de 1 microsegundo
    if ((_txCounter++ % 13) != 0) return;

    // Guarda de segurança: rádio ainda ocupado com o pacote anterior
    // Não deve ocorrer em condições normais (130ms > 40ms tempo no ar)
    if (_txBusy) return;

    // Preenche _packet[16] com os 4 campos definidos
    _buildPacket(data);

    // Tenta adquirir o barramento SPI por até 5ms
    // O SDCard segura o SPI por menos de 1ms, então 5ms é mais que suficiente
    if (xSemaphoreTake(_spiMutex, pdMS_TO_TICKS(5))) {
        // startTransmit() faz apenas escritas de registro (~100µs) e retorna imediatamente
        // O rádio transmite os 16 bytes no ar sozinho, sem ocupar a CPU
        int state = _radio.startTransmit(_packet, 16);
        xSemaphoreGive(_spiMutex); // SPI livre imediatamente após iniciar o envio

        if (state == RADIOLIB_ERR_NONE)
            _txBusy = true; // aguarda ISR sinalizar quando terminar
    }
    // Se não conseguiu o mutex em 5ms, pula este ciclo silenciosamente
}

void LORAMODULE::_buildPacket(const FlightData& d) {
    _writeU32(_packet,  0, d.timestamp);  // ms desde o boot
    _writeF32(_packet,  4, d.latitude);   // graus decimais
    _writeF32(_packet,  8, d.longitude);  // graus decimais
    _writeF32(_packet, 12, d.altitude);   // metros acima do ponto de lançamento
}

// Escreve 4 bytes de uint32_t no offset (o) do buffer (b)
void LORAMODULE::_writeU32(uint8_t* b, int o, uint32_t v) { memcpy(b + o, &v, 4); }

// Escreve 4 bytes de float no offset (o) do buffer (b)
void LORAMODULE::_writeF32(uint8_t* b, int o, float    v) { memcpy(b + o, &v, 4); }
