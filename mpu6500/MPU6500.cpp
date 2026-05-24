#include "MPU6500.h"
#include "../config.h"

bool MPU6500::init() {
    MPU9250Setting setting;
    setting.accel_fs_sel     = ACCEL_FS_SEL::A16G;
    setting.gyro_fs_sel      = GYRO_FS_SEL::G500DPS;
    setting.gyro_dlpf_cfg    = GYRO_DLPF_CFG::DLPF_10HZ;
    setting.accel_dlpf_cfg   = ACCEL_DLPF_CFG::DLPF_10HZ;
    setting.fifo_sample_rate = FIFO_SAMPLE_RATE::SMPL_125HZ;

    if (!_sensor.setup(MPU9250_ADDR, setting)) return false;
    _sensor.calibrateAccelGyro();
    return true;
}

bool MPU6500::read(FlightData& data) {
    if (!_sensor.update()) return false;
    float ax = _sensor.getAccX();
    float ay = _sensor.getAccY();
    float az = _sensor.getAccZ();
    float gx = _sensor.getGyroX();
    float gy = _sensor.getGyroY();
    float gz = _sensor.getGyroZ();
    if (isnan(ax) || isnan(ay) || isnan(az) || isnan(gx) || isnan(gy) || isnan(gz))
        return false;
    data.accel = {ax, ay, az};
    data.gyro  = {gx, gy, gz};
    return true;
}
