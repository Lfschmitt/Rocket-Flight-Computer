#include "DataPrint.h"

bool DataPrint::printFlightData(FlightData& data) {
    
    _out.println("=== FLIGHT DATA ===");
    _out.print("Time:       "); _out.print(data.timestamp / 1000.0, 2); _out.println(" s");
    _out.println();

    _out.println("[BMP280]");
    _out.print("Altitude:   "); _out.print(data.altitude,      2); _out.println(" m");
    _out.print("Pressure:   "); _out.print(data.pressure/1000, 2); _out.println(" kPa");
    _out.print("Temp:       "); _out.print(data.temperature,   2); _out.println(" C");
    _out.println();
    _out.println("[MPU6500]");
    _out.print("Accel   X: "); _out.print(data.accel.x, 2);
    _out.print(" g    Y: ");   _out.print(data.accel.y, 2);
    _out.print(" g    Z: ");   _out.print(data.accel.z, 2); _out.println(" g");
    _out.print("Gyro    X: "); _out.print(data.gyro.x, 2);
    _out.print(" d/s  Y: ");   _out.print(data.gyro.y, 2);
    _out.print(" d/s  Z: ");   _out.print(data.gyro.z, 2); _out.println(" d/s");

    _out.println();
    _out.println("[GPS]");
    _out.print("Fix:        "); _out.println(data.gpsFix ? "YES" : "NO");
    _out.print("Satellites: "); _out.println(data.satellites);
    if(data.gpsFix){
        _out.print("Latitude:   "); _out.println(data.latitude,  6);
        _out.print("Longitude:  "); _out.println(data.longitude, 6);
        _out.print("Maps:       https://maps.google.com/?q=");
        _out.print(data.latitude, 6);
        _out.print(",");
        _out.println(data.longitude, 6);
    }
    _out.println("===================");
    
    return true;
}