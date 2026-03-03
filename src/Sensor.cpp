#include "Sensor.h"

#include <Arduino.h>

HB9HCR_Sensor::HB9HCR_Sensor() {}

void HB9HCR_Sensor::init() {
    while (!Serial) delay(1000);

    Serial.print("BMI160 initialization ");
    if (0 == this->BMI160.I2cInit(0x69)) {
        Serial.print("complete, ");
        Serial.print("calibration ");
        int samples = 200;
        float gyr = 0;
        float acc = 0;

        for (int i = 0; i < samples; i++) {
            int16_t raw[6];
            this->BMI160.getAccelGyroData(raw);
            gyr += (raw[1] / 131.0);
            acc += (raw[3] / 16384.0);
            delay(2);
        }

        this->gyr_bias = gyr / samples;
        this->acc_bias = acc / samples;
        Serial.println("done");
    } else {
        Serial.println("fail");
    }

    Serial.print("BMM350 initialization ");
    if (0 == this->BMM350.begin()) {
        this->BMM350.setOperationMode(eBmm350NormalMode);
        this->BMM350.setPresetMode(BMM350_PRESETMODE_HIGHACCURACY, BMM350_DATA_RATE_25HZ);
        this->BMM350.setMeasurementXYZ();
        Serial.println("complete");
    } else {
        Serial.println("fail");
    }

    Serial.print("INA219 initialization ");
    if (this->INA219.begin()) {
        Serial.println("complete");
    } else {
        Serial.println("fail");
    }
}

void HB9HCR_Sensor::dump() {
    if (!HB9HCR_SENSOR_DEBUG || millis() - this->last[0] < 1000) return;
    this->last[0] = millis();

    Serial.print("BMI160 acc x,y,z=(");
    Serial.print(this->acc[0]);
    Serial.print(",");
    Serial.print(this->acc[1]);
    Serial.print(",");
    Serial.print(this->acc[2]);
    Serial.print(") gyr x,y,z=(");
    Serial.print(this->gyr[0]);
    Serial.print(",");
    Serial.print(this->gyr[1]);
    Serial.print(",");
    Serial.print(this->gyr[2]);
    Serial.print(") elevation=");
    Serial.println(this->elevation);

    Serial.print("BMM350 mag x,y,z=(");
    Serial.print(this->mag[0]);
    Serial.print(",");
    Serial.print(this->mag[1]);
    Serial.print(",");
    Serial.print(this->mag[2]);
    Serial.print(") temperature=");
    Serial.print(this->temperature);
    Serial.print(" azimuth=");
    Serial.println(this->azimuth);

    Serial.print("INA219 voltage bus,shunt=(");
    Serial.print(this->ina[0]);
    Serial.print(",");
    Serial.print(this->ina[1]);
    Serial.print(") current=");
    Serial.print(this->ina[2]);
    Serial.print(" power=");
    Serial.println(this->ina[3]);
}

void HB9HCR_Sensor::read(HB9HCR_Computer* computer) {
    float dt = (millis() - this->last[1]) / 1000.0;
    this->last[1] = millis();

    // acceleration & gyrometer = elevation
    if (!computer->mem[10]) {
        int16_t data_bmi160[6];
        this->BMI160.getAccelGyroData(data_bmi160);
        this->acc[0] = (data_bmi160[3] / 16384.0);  // - this->acc_bias;
        this->acc[1] = (data_bmi160[4] / 16384.0);
        this->acc[2] = (data_bmi160[5] / 16384.0);
        this->gyr[0] = (data_bmi160[0] / 131.0);
        this->gyr[1] = (data_bmi160[1] / 131.0) - this->gyr_bias;
        this->gyr[2] = (data_bmi160[2] / 131.0);

        float acc = atan2(-this->acc[0], sqrt(this->acc[1] * this->acc[1] + this->acc[2] * this->acc[2])) * 57.29577;
        this->elevation = HB9HCR_SENSOR_COMPLEMENT * (this->elevation + this->gyr[1] * dt) + (1.0 - HB9HCR_SENSOR_COMPLEMENT) * (acc);
    }

    // magnetometer = azimuth & temperature
    if (!computer->mem[11]) {
        sBmm350MagData_t data_bmm350 = this->BMM350.getGeomagneticData();
        this->azimuth = fmod(360.0 - this->BMM350.getCompassDegree(), 360.0);
        this->azimuth = 0 > this->azimuth ? this->azimuth + 360.0 : this->azimuth;
        this->temperature = data_bmm350.float_temperature;
        this->mag[0] = data_bmm350.float_x;
        this->mag[1] = data_bmm350.float_y;
        this->mag[2] = data_bmm350.float_z;
    }

    // wattmeter = voltage, current & power
    if (!computer->mem[12]) {
        this->ina[0] = this->INA219.getBusVoltage_V() * 1000;
        this->ina[1] = this->INA219.getShuntVoltage_mV();
        this->ina[2] = this->INA219.getCurrent_mA();
        // this->ina[3] = this->INA219.getPower_mW(); // did DFRobot forget to handle division / zero?
    }
}

void HB9HCR_Sensor::saturate(HB9HCR_Computer* computer) {
    // raw
    computer->mem[20] = this->acc[0];
    computer->mem[21] = this->acc[1];
    computer->mem[22] = this->acc[2];
    computer->mem[23] = this->gyr[0];
    computer->mem[24] = this->gyr[1];
    computer->mem[25] = this->gyr[2];
    computer->mem[26] = this->mag[0];
    computer->mem[27] = this->mag[1];
    computer->mem[28] = this->mag[2];

    // processed
    computer->mem[30] = this->ina[0];
    computer->mem[31] = this->ina[1];
    computer->mem[32] = this->ina[2];
    computer->mem[33] = this->ina[3];
    computer->mem[34] = this->temperature;
    computer->mem[35] = this->azimuth;
    computer->mem[36] = this->elevation;
}