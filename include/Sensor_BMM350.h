#ifndef __HB9HCR_BMM350__
#define __HB9HCR_BMM350__

#include <DFRobot_BMM350.h>

class HB9HCR_BMM350 {
   private:
    DFRobot_BMM350_I2C Sensor = DFRobot_BMM350_I2C(&Wire);

   public:
    struct data {
        float x;
        float y;
        float z;
    };

    sBmm350MagData_t raw;
    data calibrated, offset;
    data scale = {1.0, 1.0, 1.0};

    HB9HCR_BMM350() {}

    void begin() {
        Sensor.begin();
        delay(100);
        Sensor.softReset();
        delay(100);
        Sensor.setOperationMode(eBmm350NormalMode);
        Sensor.setPresetMode(BMM350_PRESETMODE_ENHANCED);
        Sensor.setMeasurementXYZ();
    }

    HB9HCR_BMM350* read() {
        // raw, but temperature compensated
        raw = Sensor.getGeomagneticData();

        // calibrated
        calibrated.x = (raw.x - offset.x) * scale.x;
        calibrated.y = (raw.y - offset.y) * scale.y;
        calibrated.z = (raw.z - offset.z) * scale.z;

        return this;
    }

    float heading() {
        float h = atan2(calibrated.y, calibrated.x) * (180.0 / PI);
        return h < 0 ? h + 360.0 : h;
    }

    void calibrate(unsigned int timeout = 10000) {
        // read initial
        read();
        float x_min = raw.x, x_max = raw.x;
        float y_min = raw.y, y_max = raw.y;
        float z_min = raw.z, z_max = raw.z;
        int s = 0, t = millis();

        // take samples
        while (millis() - t < timeout) {
            read();
            x_min = raw.x < x_min ? raw.x : x_min;
            x_max = raw.x > x_max ? raw.x : x_max;
            y_min = raw.y < y_min ? raw.y : y_min;
            y_max = raw.y > y_max ? raw.y : y_max;
            z_min = raw.z < z_min ? raw.z : z_min;
            z_max = raw.z > z_max ? raw.z : z_max;
            s++;
        }

        // set offsets
        offset.x = (x_max + x_min) / 2.0f;
        offset.y = (y_max + y_min) / 2.0f;
        offset.z = (z_max + z_min) / 2.0f;

        // calculate range
        float x_rng = x_max - x_min;
        float y_rng = y_max - y_min;
        float z_rng = z_max - z_min;
        float a_rng = (y_rng + z_rng) / 2.0f;

        // set scale
        scale.x = a_rng / x_rng;
        scale.y = a_rng / y_rng;
        scale.z = a_rng / z_rng;
    }
};

#endif