#pragma once
#include "../FlightData.h"
#include <Print.h>

class DataPrint {
public:
    DataPrint(Print& out) : _out(out) {}
    bool printFlightData(FlightData& data);

private:
    Print& _out;
};
