//Esse arquivo é para declarar todas as funções que o sensor GYGPS faz
#pragma once                //Inclui esse arquivo apenas uma vez
#include <TinyGPSPlus.h>    //Biblioteca para ler o sensor
#include "../FlightData.h"  //Necessário para incluir o objeto FlightData
#include "../config.h"      //Busca as configurações do hardware

//Cria a classe GYGPS
class GYGPS{
    //Cria as funções publicas para a main acessar
    public:
        bool init(HardwareSerial& serial);  //Função para inicializar os dados
        void feed(uint8_t byte);            //Função para receber todos os bytes do sensor
        bool read(FlightData& data);        //Função para converter os bytes em informação
    //Cria as funções privadas para o sensor acessar
    private:
        TinyGPSPlus _gygps;         //Cria o objeto do sensor GYGPS
        HardwareSerial* _serial;    //Cria um objeto para o serial 
};