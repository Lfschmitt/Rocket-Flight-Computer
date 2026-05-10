/*****************************************************\
    Esse arquivo é um struct que cria variáveis
    para todos os dados importantes de voo 
\*****************************************************/
#pragma once            //Inclue esse arquivo apenas uma vez
#include <stdint.h>     //Importa variáveis com tamanho fixo (uint32_t)

//Define um struct para vetor
struct Vector3 {
    float x, y, z;
};
//Define o struct para os dados de voo
struct FlightData {
    uint32_t timestamp;         // Tempo desde o boot (ms)

    // Dados fornecidos pelo BMP280
    float altitude;             // Metros acima do ponto de lançamento    (m)
    float pressure;             // Pressão atmosférica externa ao foguete (Pa)
    float temperature;          // Temperatura externa ao foguete         (°C)

    //Dados fornecidos pelo MPU6500
    Vector3 accel;              // Aceleração em g-forces           (g)
    Vector3 gyro;               // Velocidade de rotação do foguete (deg/s)

    //Dados fornecidos pelo GPS
    float latitude;
    float longitude;
    float gpsAltitude;
    float gpsHDOP;      //Horizontal Dilution of Precision: 
                        //Qualidade do fix (menor = melhor; <2 é excelente, >5 é ruim)
    float gpsSpeed;     //Velocidade calculada pelo GPS, em m/s
    bool  gpsFix;
    uint8_t satellites;

    // Dados processado (DataProcessor)
    float verticalVelocity;   // m/s
    float verticalAccel;      // m/s²

    //Status de cada sistema
    uint8_t systemStatus;
};
