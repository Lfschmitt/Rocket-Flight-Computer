#pragma once
#include <SD.h>
#include <freertos/semphr.h>  // SemaphoreHandle_t, xSemaphoreCreateMutex, etc.
#include "../FlightData.h"

class SDCARD{
    public:
        bool init(SemaphoreHandle_t spiMutex);
        bool log(const FlightData& data);
        bool write();
    private:
        File dataFile;
        FlightData lastData;
        TaskHandle_t sdHandle;
        //Mutex do FreeRTOS que impede acesso simultâneo pelos dois núcleos
        SemaphoreHandle_t _dataMutex;  // protege lastData
        SemaphoreHandle_t _spiMutex;   // protege o barramento SPI
        static void writeTask(void* param);
};