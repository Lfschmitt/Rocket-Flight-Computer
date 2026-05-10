#include "SystemMonitor.h"

void SystemMonitor::update(FlightData& data, bool bmp, bool mpu, bool gps, bool sd, bool lora) {
    data.systemStatus = 0;
    if (bmp)  data.systemStatus |= SYS_BMP;
    if (mpu)  data.systemStatus |= SYS_MPU;
    if (gps)  data.systemStatus |= SYS_GPS;
    if (sd)   data.systemStatus |= SYS_SD;
    if (lora) data.systemStatus |= SYS_LORA;
}
