#pragma once
#include "../FlightData.h"
#include "RadioLib.h"
#include <freertos/semphr.h>
#include "../config.h"

class LORAMODULE {
    public:
        // Inicializa o rádio SX1276 com frequência, SF, BW e CR definidos no .cpp.
        // Recebe o spiMutex compartilhado com o SDCard para proteger o barramento SPI.
        bool init(SemaphoreHandle_t spiMutex);
        
        // Chamado a cada ciclo do loop principal (100Hz).
        // Internamente decide se é hora de transmitir com base no _txCounter.
        bool update(const FlightData& data);
        
        // Retorna o código de erro RadioLib da última tentativa de init().
        // Chamar só após init() retornar false para diagnosticar o motivo da falha.
        int getLastError() const { return _lastError; }


    private:
        // Objeto RadioLib que representa o chip SX1276 fisicamente soldado no Heltec.
        // Parâmetros: CS, DIO0 (interrupção TX), RST, DIO1 (não usado), barramento SPI.
        // RADIOLIB_NC = "não conectado" — ocupa o lugar do DIO1 que não usamos.
        SX1276 _radio = new Module(LORA_CS, LORA_DIO0, LORA_RST, RADIOLIB_NC, SPI);

        // Mesmo mutex passado para o SDCard — garante acesso exclusivo ao barramento SPI.
        SemaphoreHandle_t _spiMutex;

        // Buffer de 17 bytes: timestamp (4) + latitude (4) + longitude (4) + altitude (4) + systemStatus (1)
        uint8_t _packet[17];

        // Conta os ciclos do loop. A transmissão ocorre quando _txCounter % 13 == 0,
        // ou seja, a cada 130ms (13 ciclos × 10ms), resultando em ~7 pacotes por segundo.
        uint16_t _txCounter = 0;

        // Impedindo disparar um novo envio antes do anterior terminar.
        bool _txBusy = false;

        // Armazena o código de retorno do RadioLib da última tentativa de init().
        // Útil para diagnóstico: impresso na mensagem de erro se todas as tentativas falharem.
        int _lastError = 0;

        // Escrito como true pela ISR _onTxDone() quando o rádio termina de transmitir.
        // volatile: o compilador não pode cachear — o valor muda fora do fluxo normal (na ISR).
        // static: ISR é função C pura, não tem acesso a "this", precisa ser membro de classe.
        static volatile bool _txDone;

        // Callback da interrupção do pino DIO0 — chamado pelo hardware quando TX termina.
        // IRAM_ATTR: obrigatório no ESP32; força a função a ficar na RAM interna,
        // pois interrupções não podem executar código armazenado na flash.
        // static: mesmo motivo do _txDone acima.
        static void IRAM_ATTR _onTxDone();

        // Serializa timestamp, latitude, longitude e altitude no buffer _packet[16].
        void _buildPacket(const FlightData& data);

        // Helpers de serialização: escrevem um valor no offset (o) dentro do buffer (b).
        // Usam memcpy internamente para evitar undefined behavior de pointer aliasing.
        void _writeU32(uint8_t* b, int o, uint32_t v);
        void _writeF32(uint8_t* b, int o, float    v);
};