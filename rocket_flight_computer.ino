/*******************************************************\
    Software do foguete de 1km, projetado para o        
    foguete Meteora da missão Gorilla
    Desenvolvido por Volkswangen T30🐇 e Claudio T??💻
\*******************************************************/

#include <Wire.h>
#include <SPI.h>
#include "FlightData.h"
#include "config.h"
#include "bmp280/BMP280.h"
#include "mpu6500/MPU6500.h"
#include "sdcard/SDCard.h"
#include "gps/GYGPS.h"
#include "processing/DataPrint.h"
#include "processing/SystemMonitor.h"
#include "telemetry/LoRa.h"

//Cria objetos para os drivers (Sensores)
BMP280     bmp280;  //Barometro
MPU6500    mpu6500; //Acelerometro
GYGPS      gygps;   //GPS
//Cria objeto para o cartão SD
SDCARD sd;
//Cria objeto para o módulo LoRa
LORAMODULE lora;
//Cria objetos para arquivos de processamento
DataPrint  dataPrint(Serial);
SystemMonitor monitor;
//Cria objeto para o arquivo de dados de voo
FlightData flightData;
// Mutex compartilhado entre SD e LoRa para proteger o barramento SPI.
// Criado aqui no .ino, pois pertence ao sistema, não a um módulo específico.
SemaphoreHandle_t spiMutex;

void setup() {
    Serial.begin(115200);           //inicializa o monitor serial
    Wire.begin(PIN_SDA, PIN_SCL);   //Inicializa a comunicaçao I2C
    
    // ###################################################################### //
    //Exibe que o sistema começou a funcionar 
    pinMode(PIN_LED, OUTPUT);       //Acende o Led indicando que o setup começou
    digitalWrite(PIN_LED, HIGH);
    Serial.print("[System] Init system");
    for(int i = 0; i < 3; i++){
        digitalWrite(PIN_LED, LOW);
        Serial.print(".");
        delay(500);
        digitalWrite(PIN_LED, HIGH);
    }
    Serial.println("");
    // ###################################################################### //

    //Inicialização do sensor BMP280(Barômetro)
    if (!bmp280.init()) { 
        Serial.println("[ERROR] BMP280 init failed"); 
    }else{Serial.println("[OK] BMP280 initialized");}
    //Inicialização do sensor MPU6500(Acelerometro)
    if (!mpu6500.init()) {
        Serial.println("[ERROR] MPU6500 init failed");
    }else{Serial.println("[OK] MPU6500 initialized");}
    
    // Inicializa o barramento SPI uma única vez antes de qualquer módulo SPI.
    // O SPI é disputado pelo LoRa e pelo cartão SD, então é necessário usar FreeRTOS
    // para que ambos os sistemas conversem entre si sem competição
    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    
    // Cria o mutex SPI antes de qualquer módulo que usa o barramento.
    spiMutex = xSemaphoreCreateMutex();

    // Mantém o CS do SD em HIGH enquanto o LoRa inicializa,
    // evitando que o cartão SD interfira no barramento SPI durante lora.init()
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);
    //Inicialização do módulo LoRa
    if (!lora.init(spiMutex)) {
        Serial.print("[ERROR] LoRa falhou apos 3 tentativas — state=");
        Serial.println(lora.getLastError());
    }else{Serial.println("[OK] LoRa initialized");}
    
    // Mantém o CS do LoRa em HIGH enquanto o SD inicializa,
    // evitando que o LoRa interfira no barramento SPI durante SD.begin()
    pinMode(LORA_CS, OUTPUT);
    digitalWrite(LORA_CS, HIGH);
    //Inicialização do cartão SD
    
    if (sd.init(spiMutex)) {
        Serial.println("[ERROR] SDCard init failed: Not Begin");
    }else {Serial.println("[OK] SDCard initialized");}
    delay(100);
    
    //Inicialização do GYGPS
    if (!gygps.init(Serial2)) {
        Serial.println("[ERROR] GPS init failed");
    }else{Serial.println("[OK] GPS initialized");}

    //Tudo certo para prosseguir o sistema
    Serial.println("[System] Flight computer ready");
    digitalWrite(PIN_LED, LOW);
    delay(3000);
    digitalWrite(PIN_LED, HIGH);
}

void loop() {
    //Atualiza o tempo da operação do sistema
    uint32_t cycleStart = millis();
    flightData.timestamp = cycleStart;

    //Le todos os sensores
    bool bmp_status = bmp280.read(flightData);
    bool mpu_status = mpu6500.read(flightData);    
    while(Serial2.available()>0){
        gygps.feed(Serial2.read());
    }
    bool gps_status = gygps.read(flightData);
    //Registra os dados e envia-os via LoRa
    bool sd_status = sd.log(flightData);
    bool lora_status = lora.update(flightData);

    //Essa linha é para debugar o sistema, em modo de operação ela deve estar comentada
    //dataPrint.printFlightData(flightData);
    monitor.update(flightData, bmp_status, mpu_status, gps_status, sd_status, 1);

    //Padroniza o tempo de loop, para que todos sensores tenham a mesma quantidade de leituras
    uint32_t elapsed = millis() - cycleStart;
    if (elapsed < LOOP_PERIOD_MS) {
        delay(LOOP_PERIOD_MS - elapsed);
    }
}
