#pragma once
#include "../FlightData.h"
#include "../config.h"

// Monitora o status dos periféricos e atualiza o campo systemStatus em FlightData
// como um bitmask — cada bit representa um periférico (ver SYS_* em config.h)
class SystemMonitor {
    public:
        // Recebe o status da leitura do ciclo atual de cada periférico e consolida em FlightData
        // Chamado a cada iteração do loop principal, então o bitmask reflete o estado corrente
        void update(FlightData& data, bool bmp, bool mpu, bool gps, bool sd, bool lora);
};
