#pragma once
#include <MPU9250.h>
#include "../FlightData.h"
#include "../config.h"

class MPU6500 {
    public:
        bool init();
        bool read(FlightData& data);

    private:
        MPU9250 _sensor;
};
