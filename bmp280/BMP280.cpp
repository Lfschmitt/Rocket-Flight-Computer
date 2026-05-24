#include "BMP280.h"     //Inclue o objeto sensor
#include "../config.h"  //Busca as configurações do hardware
#include <Arduino.h>

//Função inicializadora do sensor
bool BMP280::init() {
    //Verifica se o endereço do sensor foi encontrado
    if (!_sensor.begin(BME280_ADDR)) {
        return false;
    }
    //Define a configuração inicial do sensor (Standard)
    _sensor.setSampling(
        Adafruit_BME280::MODE_NORMAL,
        Adafruit_BME280::SAMPLING_X2,   // temperatura
        Adafruit_BME280::SAMPLING_X16,  // pressão
        Adafruit_BME280::SAMPLING_X1,   // umidade
        Adafruit_BME280::FILTER_X16,
        Adafruit_BME280::STANDBY_MS_0_5
    );

    // Média de amostras para pressão de referência do solo
    float sum = 0;
    for (int i = 0; i < BME280_CALIBRATION_SAMPLES; i++) {
        sum += _sensor.readPressure();
        delay(10);
    }
    _referencePressure = sum / BME280_CALIBRATION_SAMPLES;
    //Retorna que o sensor foi inicializado corretamente
    return true;
}
//Função para ler e armazenar os dados no FlightData
bool BMP280::read(FlightData& data) {
    //Le a pressão e a temperatura
    data.pressure    = _sensor.readPressure();
    data.temperature = _sensor.readTemperature();

    //Retorna false se os valores forem absurdos
    if(data.pressure <= 0.0f || data.temperature <= -30.0f)
        return false;

    // Altitude relativa ao ponto de lançamento
    // Utilizando a Equação Barométrica Hipsométrica
    data.altitude = 44330.0f * (1.0f - pow(data.pressure / _referencePressure, 0.1903f));
    return true;
}
