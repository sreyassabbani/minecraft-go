#ifndef GYRO_H
#define GYRO_H

#include "algebra.h"
#include <MPU6050.h>
#include <Arduino.h> // for micros() and types if needed

using algebra::Vector;
using algebra::Quaternion;

class Gyro {
public:
    Gyro();

    void begin();
    
    // Updates the filter and state. Should be called frequently.
    void update();

    // Getters
    Vector<3> getLinearAcceleration() const;
    Vector<3> getOrientationEuler() const;
    Vector<3> getPosition() const;
    Vector<3> getVelocity() const;

private:
    MPU6050 mpu;

    // --- Sensor scale factors ---
    float accelLsbPerG = 16384.0f;     // ±2g
    float gyroLsbPerDPS = 16.4f;       // ±2000 °/s
    const float G0 = 9.80665f;

    // --- Orientation state (Mahony filter) ---
    Quaternion q{1.0f, 0.0f, 0.0f, 0.0f};      // world <- body
    float twoKp = 2.0f * 0.5f;                  // proportional gain
    float twoKi = 2.0f * 0.0f;                  // integral gain
    Vector<3> integralFB{0.0f, 0.0f, 0.0f};     // integral feedback term
    Vector<3> gyroBias{0.0f, 0.0f, 0.0f};       // software bias (rad/s)

    // --- Position/velocity in world frame ---
    Vector<3> position{0.0f, 0.0f, 0.0f};
    Vector<3> velocity{0.0f, 0.0f, 0.0f};
    Vector<3> linearAccelWorld{0.0f, 0.0f, 0.0f}; // Store last calculated linear accel

    // --- Timing ---
    unsigned long lastMicros = 0;

    // --- Drift mitigation thresholds ---
    const float stillAccelThresh_g = 0.02f;
    const float stillGyroThresh_dps = 1.0f;
    const float velDamping = 0.999f;

    // Internal helper
    void mahonyUpdate(float gx_rad, float gy_rad, float gz_rad,
                      float ax, float ay, float az, float dt);
};

#endif // GYRO_H
