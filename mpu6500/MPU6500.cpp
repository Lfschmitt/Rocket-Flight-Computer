#include "MPU6500.h"    //Inclue o objeto sensor

//Função inicializadora do sensor
bool MPU6500::init() {
    //Define a configuração inicial do sensor
    MPU9250Setting setting;
    setting.accel_fs_sel     = ACCEL_FS_SEL::A8G;            // escala do acelerômetro: ±8g
    setting.gyro_fs_sel      = GYRO_FS_SEL::G500DPS;         // escala do giroscópio: ±500°/s
    setting.gyro_dlpf_cfg    = GYRO_DLPF_CFG::DLPF_10HZ;     // filtro passa-baixa do giroscópio
    setting.accel_dlpf_cfg   = ACCEL_DLPF_CFG::DLPF_10HZ;    // filtro passa-baixa do acelerômetro
    setting.fifo_sample_rate = FIFO_SAMPLE_RATE::SMPL_125HZ; // taxa de amostragem

    //Verifica se o endereço do sensor foi encontrado
    if (!_sensor.setup(MPU9250_ADDR, setting))
        return false;

    // Média de amostras para referência em repouso
    float sGx = 0, sGy = 0, sGz = 0;
    for (int i = 0; i < MPU9250_CALIBRATION_SAMPLES; i++) {
        _sensor.update();
        sGx += _sensor.getGyroX();
        sGy += _sensor.getGyroY();
        sGz += _sensor.getGyroZ();
        delay(20);
    }
    _refGx = sGx / MPU9250_CALIBRATION_SAMPLES;
    _refGy = sGy / MPU9250_CALIBRATION_SAMPLES;
    _refGz = sGz / MPU9250_CALIBRATION_SAMPLES;

    //Retorna que o sensor foi inicializado corretamente
    return true;
}

//Função para ler e armazenar os dados no FlightData
bool MPU6500::read(FlightData& data) {
    //Retorna false se o sensor não conseguiu atualizar os dados
    if (!_sensor.update()) 
        return false;

    //Lê os dados do acelerômetro e do giroscópio descontando a referência em repouso
    float ax = _sensor.getAccX();
    float ay = _sensor.getAccY();
    float az = _sensor.getAccZ();
    float gx = _sensor.getGyroX() - _refGx;
    float gy = _sensor.getGyroY() - _refGy;
    float gz = _sensor.getGyroZ() - _refGz;

    //Retorna false se algum valor for inválido
    if (isnan(ax) || isnan(ay) || isnan(az) || isnan(gx) || isnan(gy) || isnan(gz))
        return false;

    //Armazena os dados lidos no FlightData
    data.accel = {ax, ay, az};
    data.gyro  = {gx, gy, gz};
    return true;
}
