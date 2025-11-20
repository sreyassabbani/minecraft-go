#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include "../utils/general.h"
#include "../utils/algebra.h"

using algebra::Vector;

struct Quaternion;

class Gyro {
private:
    MPU6050 mpu;

    // Sensor scale factors
    // Accelerometer: LSB/g (change if you set a different FS range)
    float accelLsbPerG = 16384.0f;     // ±2g
    // Gyro: LSB per °/s (change if you set a different FS range)
    float gyroLsbPerDPS = 16.4f;       // ±2000 °/s
    const float G0 = 9.80665f;

    // Orientation state (Mahony filter)
    Quaternion q{1.0f, 0.0f, 0.0f, 0.0f};      // world <- body
    float twoKp = 2.0f * 0.5f;                  // proportional gain (Kp ~ 0.5)
    float twoKi = 2.0f * 0.0f;                  // integral gain (Ki; set >0 to estimate gyro bias)
    Vector<3> integralFB{0.0f, 0.0f, 0.0f};     // integral feedback term
    Vector<3> gyroBias{0.0f, 0.0f, 0.0f};       // software bias (rad/s)

    // --- Position/velocity in world frame ---
    Vector<3> position{0.0f, 0.0f, 0.0f};
    Vector<3> velocity{0.0f, 0.0f, 0.0f};

    // --- Timing ---
    unsigned long lastMicros = 0;

    // --- Drift mitigation thresholds ---
    const float stillAccelThresh_g = 0.02f;     // |accel| close to 1 g (normalized)
    const float stillGyroThresh_dps = 1.0f;     // near zero rotation
    const float velDamping = 0.999f;

    // Internal Mahony update
    void mahonyUpdate(float gx_rad, float gy_rad, float gz_rad,
                      float ax, float ay, float az, float dt);

public:
    Gyro();

    void begin();
    Vector<3> read();                  // Returns world-frame linear acceleration (m/s^2)
    Vector<3> read_orientation();      // Returns yaw/pitch/roll (radians)
    Vector<3> get_position() const;    // Returns absolute position
    Vector<3> get_velocity() const;    // Returns velocity
};

