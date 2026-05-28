//Esse arquivo é para declarar todas as funções que o sensor MPU6500 faz
//O projeto foi idealizado com o MPU6500, porém é a mesma lógica e pinagem para o MPU9250
//que era o sensor que estava disponível no momento, esse modelo tem magnetômetro, então
//tem a possibilidade de expansão de leituras

#pragma once                        //Inclue esse arquivo apenas uma vez
#include <MPU9250.h>                //Biblioteca para ler o sensor MPU9250
#include "../FlightData.h"          //Necessário para incluir o objeto FlightData
#include "../config.h"              //Busca as configurações do hardware
#include <Arduino.h>                //Adiciona funções comuns do Arduino IDE

//Cria a classe MPU6500
class MPU6500 {
    //Cria as funções publicas para a main acessar
    public:
        bool init();                    //Função para inicializar os dados
        bool read(FlightData& data);    //Função para ler os dados

    //Cria as funções privadas para o sensor acessar
    private:
        MPU9250 _sensor;                //Cria o objeto do sensor MPU9250
};
