/*****************************************************\
    Esse arquivo é especifico para o funcionamento 
    do ArduinoIDE, pois ele não lida muito bem com
    estrutura de arquivos,então precisa de um .cpp
    para inicializar todos os módulos 
\*****************************************************/

#include "bmp280/BMP280.cpp"
#include "mpu6500/MPU6500.cpp"
#include "sdcard/SDCard.cpp"
#include "gps/GYGPS.cpp"
#include "telemetry/LoRa.cpp"

// Descomente conforme os módulos forem ativados:
// #include "processing/DataProcessor.cpp"
// #include "processing/ApogeeDetector.cpp"

#include "processing/DataPrint.cpp"
#include "processing/SystemMonitor.cpp"
// #include "state/StateMachine.cpp"
