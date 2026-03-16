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
        h += 180.0;
        if (h > 360) h -= 360;
        if (h < 0) h += 360;
        return h;
    }
};

#endif