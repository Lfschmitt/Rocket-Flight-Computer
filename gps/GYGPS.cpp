#include "GYGPS.h" //Inclue o objeto sensor

//Função inicializadora do sensor
bool GYGPS::init(HardwareSerial& serial){
    //Salva a referência da porta serial que o GPS está conectado
    _serial = &serial;
    //Inicializa a comunicação serial: BAUD 9600, 8 bits de dados, sem paridade, 1 stop bit
    //Pinos RX e TX definidos no config.h
    _serial->begin(GPS_BAUD, SERIAL_8N1, PIN_RX_GPS, PIN_TX_GPS);
    return true;
}

//Função para receber os bytes que o sensor está fornecendo
//Deve ser chamada no loop principal para cada byte disponível na serial
void GYGPS::feed(uint8_t byte){
    //Passa o byte para o parser TinyGPS++ montar as sentenças NMEA
    _gygps.encode(byte);
}

//Função para interpretar os bytes que o sensor está lendo
bool GYGPS::read(FlightData& data){
    //O GPS não lê todos os valores ao mesmo tempo — ele envia sentenças NMEA parciais
    //Só considera válido quando já recebeu uma posição (lat/lon) confirmada pelo satélite
    if(_gygps.location.isValid()){
        data.gpsFix    = true;
        data.latitude  = _gygps.location.lat();   // graus decimais
        data.longitude = _gygps.location.lng();   // graus decimais
        //Os campos abaixo são opcionais: cada um só é atualizado se já chegou dado válido
        //Caso contrário, mantém o último valor conhecido no FlightData
        if(_gygps.altitude.isValid())
            data.gpsAltitude = _gygps.altitude.meters();    // metros acima do nível do mar
        if(_gygps.speed.isValid())
            data.gpsSpeed = _gygps.speed.kmph() / 3.6f;    // converte km/h → m/s
        if(_gygps.hdop.isValid())
            data.gpsHDOP = _gygps.hdop.hdop();             // qualidade do fix (menor = melhor)
        if(_gygps.satellites.isValid())
            data.satellites = _gygps.satellites.value();   // número de satélites visíveis
        return true;
    } else {
        //Sem fix: sinaliza no FlightData mas preserva a última posição conhecida
        data.gpsFix = false;
        return false;
    }
}