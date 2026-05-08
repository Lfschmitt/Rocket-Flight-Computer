#pragma once                        //Inclue esse arquivo apenas uma vez
#include <MPU6500_WE.h>             //Biblioteca para ler o sensor MPU6500
#include "../FlightData.h"
#include "../config.h"              

//Cria a classe MPU6500
class MPU6500 {
    //Cria as funções publicas para a main acessar
    public:
        bool init();                    //Retorna true(Inicializou corretamente) ou false(Não inicializou)
        bool read(FlightData& data);    

    //Cria as funções privadas para o acessar
    private:
        xyzFloat referenceAccel;
        xyzFloat referenceGyro;
        MPU6500_WE _sensor = MPU6500_WE(MPU6500_ADDR);        
};
