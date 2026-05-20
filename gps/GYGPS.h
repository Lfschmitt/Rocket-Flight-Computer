#pragma once
#include <TinyGPSPlus.h>
#include "../FlightData.h"

class GYGPS{
    public:
        bool init(HardwareSerial& serial);
        void feed(uint8_t byte);
        bool read(FlightData& data);
    private:
        TinyGPSPlus _gygps;
        HardwareSerial* _serial;
};