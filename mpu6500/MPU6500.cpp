#include "MPU6500.h"    //Inclue o objeto sensor
#include "../config.h"  //Busca as configurações do hardware
#include <Arduino.h>

//Função inicializadora do sensor
bool MPU6500::init() {
    //Define a configuração inicial do sensor
    MPU9250Setting setting;
    setting.accel_fs_sel     = ACCEL_FS_SEL::A16G;         // escala do acelerômetro: ±16g
    setting.gyro_fs_sel      = GYRO_FS_SEL::G500DPS;       // escala do giroscópio: ±500°/s
    setting.gyro_dlpf_cfg    = GYRO_DLPF_CFG::DLPF_10HZ;  // filtro passa-baixa do giroscópio
    setting.accel_dlpf_cfg   = ACCEL_DLPF_CFG::DLPF_10HZ; // filtro passa-baixa do acelerômetro
    setting.fifo_sample_rate = FIFO_SAMPLE_RATE::SMPL_125HZ; // taxa de amostragem

    //Verifica se o endereço do sensor foi encontrado
    if (!_sensor.setup(MPU9250_ADDR, setting)) return false;
    //Calibra o acelerômetro e o giroscópio com o foguete em repouso
    _sensor.calibrateAccelGyro();
    //Retorna que o sensor foi inicializado corretamente
    return true;
}

//Função para ler e armazenar os dados no FlightData
bool MPU6500::read(FlightData& data) {
    //Retorna false se o sensor não conseguiu atualizar os dados
    if (!_sensor.update()) return false;

    //Lê os dados do acelerômetro e do giroscópio
    float ax = _sensor.getAccX();
    float ay = _sensor.getAccY();
    float az = _sensor.getAccZ();
    float gx = _sensor.getGyroX();
    float gy = _sensor.getGyroY();
    float gz = _sensor.getGyroZ();

    //Retorna false se algum valor for inválido
    if (isnan(ax) || isnan(ay) || isnan(az) || isnan(gx) || isnan(gy) || isnan(gz))
        return false;

    //Armazena os dados lidos no FlightData
    data.accel = {ax, ay, az};
    data.gyro  = {gx, gy, gz};
    return true;
}
