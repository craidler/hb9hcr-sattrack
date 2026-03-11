#ifndef __HB9HCR_ACTUATOR__
#define __HB9HCR_ACTUATOR__

#include <SCServo.h>

#define HB9HCR_AXIS_AZ 1
#define HB9HCR_AXIS_EL 2

class HB9HCR_Actuator {
   private:
    static constexpr float spd = 4096.0f / 360.0f;
    SMS_STS Servo;

   public:
    uint16_t el_center = 1023;
    int az_position = 0;
    int el_position = 0;
    float az_current = 0;
    float el_current = 0;
    float az_offset = 0;
    float el_offset = 0;
    bool az_moving = false;
    bool el_moving = false;

    HB9HCR_Actuator() {}

    void begin() {
        Serial0.begin(1000000, SERIAL_8N1, RX, TX);

        while (!Serial0) {
            delay(500);
        }

        Servo.pSerial = &Serial0;

        /*
        Servo.unLockEprom(1);

        // 2. Write the new ID (2) to the ID register of servo 1
        // Parameters: CurrentID, Register, NewValue
        Servo.writeByte(1, SMS_STS_ID, 2);

        // 3. Lock the EPROM to save the change
        Servo.LockEprom(2);

        Serial.println("Change complete. Verifying...");

        delay(1000);
        if (Servo.FeedBack(2) != -1) {
            Serial.println("SUCCESS: Servo is now ID 2!");
        } else {
            Serial.println("FAILED: Servo not found at ID 2. Check power/wiring.");
        }
        */

        /*
        Servo.WritePosEx(1, 1535, 0, 50);
        Servo.WritePosEx(2, 1535, 0, 50);
        delay(2000);
        Servo.WritePosEx(1, 511, 0, 50);
        Servo.WritePosEx(2, 511, 0, 50);
        delay(2000);
        Servo.WritePosEx(1, 1023, 0, 50);
        Servo.WritePosEx(2, 1023, 0, 50);
        */

        /*
        Servo.unLockEprom(1); // Unlock to allow changes
        Servo.writeByte(1, SMS_STS_MODE, 3); // 0 = Servo Mode, 1 = Motor Mode, 2, 3 = Step Mode?
        Servo.LockEprom(1); // Lock to save
        Servo.EnableTorque(1, 1);
        */

        /*
        Servo.WritePosEx(1, 256, 0, 25);
        delay(1000);
        Servo.WritePosEx(1, -512, 0, 25);
        delay(1000);
        Servo.WritePosEx(1, 256, 0, 25);
        */
    }

    void home(float az, float el) {
        this->az_current = az;
        this->el_current = el;
        this->el_offset = -el;
        this->move(0.0f, 0.0f);
    }

    void moveAz(float t) {
        if (this->az_moving) return;
        this->az_moving = true;
        Servo.WritePosEx(HB9HCR_AXIS_AZ, constrain(this->shortest(this->az_current, t) * this->spd, -4095.0, 4095.0), 0, 25);
        delay(1000);
        this->az_current = t;
        this->az_moving = false;
    }

    void moveEl(float t) {
        if (this->el_moving) return;
        this->el_moving = true;
        Servo.WritePosEx(HB9HCR_AXIS_EL, constrain(this->el_center + (this->el_offset + t) * this->spd, 0, 2047), 0, 25);
        delay(1000);
        this->el_current = t;
        this->el_moving = false;
    }

    void move(float az, float el) {
        if (this->az_moving || this->el_moving) return;
        this->az_moving = this->el_moving = true;
        Servo.WritePosEx(HB9HCR_AXIS_AZ, constrain(this->shortest(this->az_current, az) * this->spd, -4095.0, 4095.0), 0, 25);
        Servo.WritePosEx(HB9HCR_AXIS_EL, constrain(this->el_center + (this->el_offset + el) * this->spd, 0, 2047), 0, 25);
        delay(1000);
        this->az_current = az;
        this->el_current = el;
        this->az_moving = this->el_moving = false;
    }

    void read() {
        this->el_position = Servo.ReadPos(HB9HCR_AXIS_EL);
        this->az_position = Servo.ReadPos(HB9HCR_AXIS_AZ);
    }

    float shortest(float current, float target) {
        float diff = target - current;
        while (diff <= -180) diff += 360;
        while (diff > 180) diff -= 360;
        return diff;
    }
};

#endif