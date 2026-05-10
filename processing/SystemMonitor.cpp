#include "SystemMonitor.h"

void SystemMonitor::update(FlightData& data, bool bmp, bool mpu, bool gps, bool sd, bool lora) {
    data.systemStatus = 0; // Zera o bitmask antes de reconstruí-lo

    // Liga o bit correspondente a cada periférico que inicializou com sucesso
    // O operador |= preserva os bits anteriores ao adicionar o novo
    if (bmp)  data.systemStatus |= SYS_BMP;  // Bit 0 — Barômetro BMP
    if (mpu)  data.systemStatus |= SYS_MPU;  // Bit 1 — IMU MPU
    if (gps)  data.systemStatus |= SYS_GPS;  // Bit 2 — GPS
    if (sd)   data.systemStatus |= SYS_SD;   // Bit 3 — Cartão SD
    if (lora) data.systemStatus |= SYS_LORA; // Bit 4 — Rádio LoRa
}