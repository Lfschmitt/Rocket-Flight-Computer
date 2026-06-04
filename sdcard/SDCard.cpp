#include "SDCard.h"
#include "../config.h"
#include "../FlightData.h"

bool SDCARD::init(SemaphoreHandle_t spiMutex){
    //Testa se o cartão SD está conectado
    if(!SD.begin(PIN_SD_CS))
        return false;

    //Abre o arquivo (FILE_APPEND) para registras as informações, caso não exista então ele cria
    dataFile = SD.open("/flightData.txt", FILE_APPEND);
    if(!dataFile)
        return false;
        
    dataFile.println("timestamp status altitude pressure temperature accelx accely accelz gyrox gyroy gyroz lat lng sats"); //Cria o cabeçalho

    // Recebe o spiMutex criado no .ino — o mesmo ponteiro é passado para o LoRa,
    // garantindo que SD e LoRa nunca usem o barramento SPI ao mesmo tempo
    _spiMutex = spiMutex;

    // _dataMutex é interno do SDCard — protege lastData entre log() (Core 1) e write() (Core 0)
    _dataMutex = xSemaphoreCreateMutex();

    //Cria uma task para processar a escrita no core 0
    xTaskCreatePinnedToCore(writeTask, "sdWriteTask", 4096, this, 1, &sdHandle, 0);

    return true;
}

bool SDCARD::log(const FlightData& data){
    // Tenta adquirir o mutex; portMAX_DELAY = espera indefinidamente até conseguir
    // Impede que log() e write() acessem lastData ao mesmo tempo em núcleos diferentes
    if(xSemaphoreTake(_dataMutex, pdMS_TO_TICKS(5))){
        lastData = data;
        xSemaphoreGive(_dataMutex); // Libera o mutex para que o outro núcleo possa acessar lastData
    }
    return true;
}

bool SDCARD::write(){
    // Copia lastData para uma variável local o mais rápido possível.
    // _dataMutex é travado só pelo tempo da cópia,  liberando log()
    // para atualizar lastData enquanto o SD ainda está escrevendo.
    FlightData localData;
    if(xSemaphoreTake(_dataMutex, pdMS_TO_TICKS(5))){
        localData = lastData;
        xSemaphoreGive(_dataMutex);
    }

    // Pscreve no SD usando os dados locais.
    // _spiMutex garante que o LoRa não usa o barramento SPI ao mesmo tempo.
    if(xSemaphoreTake(_spiMutex, pdMS_TO_TICKS(5))){
        dataFile.print(localData.timestamp); dataFile.print(", ");      //Tempo
        dataFile.print(localData.systemStatus); dataFile.print(", ");   //Status do sistema
        dataFile.print(localData.altitude); dataFile.print(", ");       //altitude
        dataFile.print(localData.pressure); dataFile.print(", ");       //pressão
        dataFile.print(localData.temperature); dataFile.print(", ");    //temperatura
        dataFile.print(localData.accel.x); dataFile.print(", ");        //accel.x
        dataFile.print(localData.accel.y); dataFile.print(", ");        //accel.y
        dataFile.print(localData.accel.z); dataFile.print(", ");        //accel.z
        dataFile.print(localData.gyro.x); dataFile.print(", ");         //gyro.x
        dataFile.print(localData.gyro.y); dataFile.print(", ");         //gyro.y
        dataFile.print(localData.gyro.z); dataFile.print(", ");         //gyro.z
        dataFile.print(localData.latitude); dataFile.print(", ");       //latitude
        dataFile.print(localData.longitude); dataFile.print(", ");      //longitude
        dataFile.println(localData.satellites);                         //Satelites
        dataFile.flush(); // flush dentro do mutex — SD ainda está no barramento SPI aqui
        xSemaphoreGive(_spiMutex);
    }
    return true;
}

void SDCARD::writeTask(void* param) {
    // param é o "this" que passado no xTaskCreatePinnedToCore
    // cast de void* para SDCARD* para ter acesso aos membros da classe
    SDCARD* self = (SDCARD*) param;

    // loop infinito — a task nunca retorna
    while (true) {
        self->write();  // chama write() na instância correta
        vTaskDelay(1);
    }
}
