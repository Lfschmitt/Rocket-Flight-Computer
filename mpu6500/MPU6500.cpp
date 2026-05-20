#include "MPU6500.h"
#include "../config.h"

bool MPU6500::init(){
    //Retorna falso se o sensor não foi encontrado
    if(!_sensor.init()) return false;

    _sensor.autoOffsets();                                 //Selecionar fonte de clock estável (PLL do giroscópio) ou setClockToGyro()
    _sensor.setAccRange(MPU6500_ACC_RANGE);                //Configurar range do acelerômetro ±16g para suportar aceleração de motor
    _sensor.setGyrRange(MPU6500_GYR_RANGE);                //Configurar range do giroscópio ±500 deg/s dependendo da rotação esperada
    _sensor.setGyrDLPF(MPU6500_DLPF);                      //Configurar filtro digital (DLPF), reduz ruído de alta frequência (vibração do motor) ~5Hz de corte
    _sensor.setSampleRateDivider(MPU6500_SAMPLE_RATE_DIV); //Configurar sample rate divider (derivado de 1kHz), SMPLRT_DIV=9 → 100Hz, coerente com LOOP_RATE_HZ
    //Média de amostras para calibrar o sensor
    for (int i = 0; i < MPU6500_CALIBRATION_SAMPLES; i++) {
        referenceAccel += _sensor.getGValues();
        referenceGyro  += _sensor.getGyrValues();
        delay(10);
    }
    referenceAccel /= MPU6500_CALIBRATION_SAMPLES;
    referenceGyro /= MPU6500_CALIBRATION_SAMPLES;
    return true;
}

bool MPU6500::read(FlightData& data){
    xyzFloat a = _sensor.getGValues()   - referenceAccel;
    xyzFloat g = _sensor.getGyrValues() - referenceGyro;
    if(isnan(a.x) || isnan(a.y) || isnan(a.z) || isnan(g.x) || isnan(g.y) || isnan(g.z))
        return false;
    data.accel = {a.x, a.y, a.z};
    data.gyro  = {g.x, g.y, g.z};
    return true;
}