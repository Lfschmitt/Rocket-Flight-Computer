//Esse arquivo é para declarar todas as funções que o sensor BMP280 faz
//O projeto foi idealizado com o BMP280, porém é a mesma lógica e pinagem para o BME280
//que era o sensor que estava disponível no momento
#pragma once                        //Inclue esse arquivo apenas uma vez
#include <Adafruit_BME280.h>        //Biblioteca para ler o sensor BME280
#include "../FlightData.h"          //Necessário para incluir o objeto FlightData
#include "../config.h"  //Busca as configurações do hardware
#include <Arduino.h>

//Cria a classe BMP280
class BMP280 {
    //Cria as funções publicas para a main acessar
    public:
        bool init();                    //Função para inicializar os dados
        bool read(FlightData& data);    //Função para ler os dados

    //Cria as funções privadas para o acessar
    private:
        Adafruit_BME280 _sensor;        //Cria o objeto do sensor BME280
        float _referencePressure;       //Variável que armazena a pressão inicial do lançamento
        float _lastSample;              //Variável que registra a ultima leitura
};
