#include "Sensor.h"

Sensor::Sensor() {
    // this->init();
}

void Sensor::init() {
    this->_bmi160.I2cInit(0x68);
    this->_bmm350.begin();
}

bool Sensor::read() {
    this->_current = micros();
    this->_dt = (this->_current - this->_last) / 1000000;
    if (this->_dt < .00001) return false;
    this->_last = this->_current;
    
    if (0 == this->_bmi160.getAccelGyroData(this->data)) {
        this->data[0] /= 16384.0;
        this->data[1] /= 16384.0;
        this->data[2] /= 16384.0;
        this->data[3] *= 2000.0 / 326768.0;
        this->data[4] *= 2000.0 / 326768.0;
        this->data[5] *= 2000.0 / 326768.0;
    }

    sBmm350MagData_t mag = this->_bmm350.getGeomagneticData();
    this->data[6] = mag.float_x;        
    this->data[7] = mag.float_y;        
    this->data[8] = mag.float_y;
    this->data[9] = mag.float_temperature;

    this->_filter.update(this->data[3], this->data[4], this->data[5], this->data[0], this->data[1], this->data[2], this->data[6], this->data[7], this->data[8]);
    this->pitch = this->_filter.getPitch();
    this->roll = this->_filter.getRoll();
    this->yaw = this->_filter.getYaw();

    return true;
}


