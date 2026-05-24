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

// BME280
#define BME280_ADDR 0x77                //Endereço do sensor BME280
#define BME280_CALIBRATION_SAMPLES  50  //Amostras para fazer a calibração inicial

//MPU9250
#define MPU9250_ADDR 0x68                       //Endereço do sensor MPU9250
#define ELECTRONIC_DISTANCE_FOR_CENTER 1        //Distância do acelerômetro ao centro de massa do foguete

//GYGPS6MV2
#define PIN_RX_GPS  36      //Pino RX do ESP32 que conecta no TX do GPS
#define PIN_TX_GPS  17      //Pino TX do Esp32 que conecta no RX do GPS
#define GPS_BAUD    9600    //Velocidade padrão do NEO-6M

// Loop principal
#define LOOP_RATE_HZ   100           //Default 100Hz
#define LOOP_PERIOD_MS (1000 / LOOP_RATE_HZ)

//Definição do bit de cada sistema
#define SYS_BMP   0x01
#define SYS_MPU   0x02
#define SYS_GPS   0x04
#define SYS_SD    0x08
#define SYS_LORA  0x10
