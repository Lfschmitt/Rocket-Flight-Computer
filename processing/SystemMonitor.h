#pragma once
#include "../FlightData.h"
#include "../config.h"

class SystemMonitor {
    public:
        void update(FlightData& data, bool bmp, bool mpu, bool gps, bool sd, bool lora);
};
