#pragma once
#include "../FlightData.h"
#include "../config.h"

// Monitora o status dos periféricos e atualiza o campo systemStatus em FlightData
// como um bitmask — cada bit representa um periférico (ver SYS_* em config.h)
class SystemMonitor {
    public:
        // Recebe o status de inicialização de cada periférico e consolida em FlightData
        void update(FlightData& data, bool bmp, bool mpu, bool gps, bool sd, bool lora);
};
