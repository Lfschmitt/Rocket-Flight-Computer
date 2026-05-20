#include "GYGPS.h"
#include "../config.h"

bool GYGPS::init(HardwareSerial& serial){
    _serial = &serial;
    _serial->begin(GPS_BAUD, SERIAL_8N1, PIN_RX_GPS, PIN_TX_GPS);
    return true;
}

void GYGPS::feed(uint8_t byte){
    _gygps.encode(byte);
}

void GYGPS::read(FlightData& data){
    if(_gygps.location.isValid()){
        data.gpsFix = true;
        data.latitude  = _gygps.location.lat();
        data.longitude = _gygps.location.lng();
        if(_gygps.altitude.isValid())
            data.gpsAltitude = _gygps.altitude.meters();
        if(_gygps.speed.isValid())
            data.gpsSpeed = _gygps.speed.kmph() / 3.6;
        if(_gygps.hdop.isValid())
            data.gpsHDOP = _gygps.hdop.hdop();
        if(_gygps.satellites.isValid())
            data.satellites = _gygps.satellites.value();
        return true;
    } else {
        data.gpsFix = false;
        return false;
    }
}