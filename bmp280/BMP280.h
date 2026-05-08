//Esse arquivo é para declarar todas as funções que o sensor BMP280 faz
#pragma once                        //Inclue esse arquivo apenas uma vez
#include <Adafruit_BMP280.h>        //Biblioteca para ler o sensor BMP280
#include "../FlightData.h"          //Necessário para incluir o objeto FlightData

//Cria a classe BMP280
class BMP280 {
    //Cria as funções publicas para a main acessar
    public:
        bool init();                    //Função para inicializar os dados
        bool read(FlightData& data);    //Função para ler os dados

    //Cria as funções privadas para o acessar
    private:
        Adafruit_BMP280 _sensor;        //Cria o objeto do sensor BMP280
        float _referencePressure;       //Variável que armazena a pressão inicial do lançamento
};
