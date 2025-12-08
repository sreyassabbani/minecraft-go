#pragma once

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <algebra.hpp>
#include <imu_sensor.hpp>

using algebra::Quaternion;
using algebra::Vector;

class Mpu6050Imu : public Imu {
public:
    Mpu6050Imu();

    void begin() override;

    // Updates the filter and state. Should be called frequently.
    void update() override;

    Vector<3> getLinearAcceleration() const override;
    Vector<3> getOrientationEuler() const override;
    Quaternion
    getOrientation(const algebra::Quaternion& current) const override;
    Vector<3> getGravityVector() const;
    Vector<3> getPosition() const override;
    Vector<3> getVelocity() const override;

private:
    Adafruit_MPU6050 mpu;

    // --- Sensor scale factors ---
    float accelLsbPerG = 16384.0f; // ±2g
    float gyroLsbPerDPS = 16.4f;   // ±2000 °/s
    const float G0 = 9.80665f;

    // --- Orientation state (gravity-based tilt) ---
    Quaternion q { 1.0f, 0.0f, 0.0f, 0.0f }; // world <- body

    // --- Position/velocity in world frame ---
    Vector<3> position { 0.0f, 0.0f, 0.0f };
    Vector<3> velocity { 0.0f, 0.0f, 0.0f };
    Vector<3> acceleration { 0.0f, 0.0f, 0.0f }; // Current acceleration sample
    Vector<3> gravity { 0.0f, 0.0f,
                        G0 }; // Raw accel (includes gravity) for orientation

    // --- Timing ---
    unsigned long lastMicros = 0;
    bool wasStill = false;

    // --- Drift mitigation thresholds ---
    const float stillAccelThresh_g = 0.02f;
    const float stillGyroThresh_dps = 1.0f;
    const float velDamping = 0.999f;
};
