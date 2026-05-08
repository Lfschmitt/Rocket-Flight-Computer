/*****************************************************\
    Esse arquivo define todos os pinos e parametros
    para o Software e para o Hardware
\*****************************************************/

#pragma once

// I2C
#define PIN_SDA  21
#define PIN_SCL  22

//SDCard
#define PIN_SD_CS 13
#define PIN_SD_MISO 19
#define PIN_SD_MOSI 27
#define PIN_SD_SCK 5

//LORA
#define LORA_CS   18
#define LORA_RST  14        //Reset módulo
#define LORA_DIO0 26        //Interrupção do TX
#define LORA_FREQ 915.0     //Essa frequência o governo brasileiro deixou liberado

// BMP280
#define BMP280_ADDR 0x77                //Endereço do sensor BMP280
#define BMP280_CALIBRATION_SAMPLES  50  //Amostras para fazer a calibração inicial

//MPU6500
#define MPU6500_ADDR 0x68                       //Endereço do sensor MPU6500
#define MPU6500_CALIBRATION_SAMPLES 50          //Amostras para fazer a calibração inicial
#define ELECTRONIC_DISTANCE_FOR_CENTER 1        //Distância do acelerômetro ao centro de massa do foguete
#define MPU6500_ACC_RANGE           MPU6500_ACC_RANGE_16G   //Define ±16g como limite
#define MPU6500_GYR_RANGE           MPU6500_GYRO_RANGE_500  //Define ±500deg/s como limite
#define MPU6500_DLPF                MPU6500_DLPF_7         //Define 10Hz como frequencia de corte, le apenas =< 10Hz 
#define MPU6500_SAMPLE_RATE_DIV     9                       //1000Hz / (1+9) = 100Hz, Sincroniza o sensor com o loop

//GYGPS6MV2
#define PIN_RX_GPS  36      //Pino RX do ESP32 que conecta no TX do GPS
#define PIN_TX_GPS  17      //Pino TX do Esp32 que conecta no RX do GPS
#define GPS_BAUD    9600    //Velocidade padrão do NEO-6M

// Loop principal
#define LOOP_RATE_HZ   100           //Default 100Hz
#define LOOP_PERIOD_MS (1000 / LOOP_RATE_HZ)
