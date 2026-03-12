#ifndef __HB9HCR_SENSOR__
#define __HB9HCR_SENSOR__

#include <DFRobot_BMI160.h>
// #include <DFRobot_BMM350.h>
#include "Actuator.h"
// #include <DFRobot_INA219.h>

class HB9HCR_Sensor {
   private:
    DFRobot_BMI160 BMI160;
    // DFRobot_BMM350_I2C BMM350 = DFRobot_BMM350_I2C(&Wire, 0x14);
    // DFRobot_INA219_IIC INA219 = DFRobot_INA219_IIC(&Wire, 0x45);
    float offset[2] = {0};
    float scale[2] = {1};

   public:
    float degree[2] = {0};

    HB9HCR_Sensor() {}

    void begin() {
        /*
        while (!this->INA219.begin()) {
            Serial.println("INA219 failed");
            delay(1000);
        }
        Serial.println("INA219 initialized");
        */

        this->BMI160.softReset();
        delay(100);

        while (this->BMI160.I2cInit(0x69)) {
            Serial.println("sensor: BMI160 failed");
            delay(500);
        }

        Serial.println("sensor: BMI160 initialized");

        /*
        this->BMM350.softReset();

        while (this->BMM350.begin()) {
            Serial.println("BMM350 failed");
            delay(1000);
        }

        this->BMM350.setPresetMode(BMM350_PRESETMODE_HIGHACCURACY);
        this->BMM350.setOperationMode(eBmm350NormalMode);
        this->BMM350.setMeasurementXYZ();
        Serial.println("BMM350 initialized");
        */
    }

    void calibrate(HB9HCR_Actuator* Actuator) {
        // take average elevation reading
        Serial.print("elevation calibration ");
        float el = 0;

        for (int i = 0; i < 200; i++) {
            this->read();
            el += this->degree[1];
            delay(5);
        }

        Serial.printf("done: %.2f\n", el /= 200);

        // initialize and cancel out elevation
        Actuator->moveTo(0, -el);
        Actuator->zero(true, true);

        // planar calibration
        /*
        Serial.print("azimuth calibration ");
        Actuator->moveTo(0, 90);
        float magMinX = 9999, magMaxX = -9999;
        float magMinY = 9999, magMaxY = -9999;

        for (int i = 0; i < 360; i++) {
            sBmm350MagData_t mag = this->BMM350.getGeomagneticData();
            if (mag.x < magMinX) magMinX = mag.x;
            if (mag.x > magMaxX) magMaxX = mag.x;
            if (mag.y < magMinY) magMinY = mag.y;
            if (mag.y > magMaxY) magMaxY = mag.y;
            Actuator->moveTo(i, 90);
            delay(20000 / 360);
        }

        // hard iron offsets
        this->offset[0] = (magMaxX + magMinX) / 2.0;
        this->offset[1] = (magMaxY + magMinY) / 2.0;

        // scaling (to make the ellipse a circle)
        float deltaX = magMaxX - magMinX;
        float deltaY = magMaxY - magMinY;
        this->scale[0] = (deltaX > deltaY) ? 1.0 : deltaY / deltaX;
        this->scale[1] = (deltaY > deltaX) ? 1.0 : deltaX / deltaY;
        Serial.printf("done: offset x,y=(%.2f, %.2f) scale x,y=(%.2f, %.2f)\n", this->offset[0], this->offset[1], this->scale[0], this->scale[1]);
        
        Actuator->moveTo(0, 0);
        Actuator->zero(true, true);
        */
    }

    void read() {
        // calculate heading
        /*
        sBmm350MagData_t mag = this->BMM350.getGeomagneticData();
        this->degree[0] = atan2((mag.y - this->offset[1]) * this->scale[1], (mag.x - this->offset[0]) * this->scale[0]) * 180.0 / PI;
        */

        // calculate elevation
        int16_t imu[6] = {0};
        this->BMI160.getAccelGyroData(imu);
        float ax_g = imu[3] / 16384.0;
        float ay_g = imu[5] / 16384.0;
        float az_g = imu[4] / 16384.0;
        this->degree[1] = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
    }
};

#endif