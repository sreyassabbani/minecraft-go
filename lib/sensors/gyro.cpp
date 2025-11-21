#include "gyro.h"

#include "algebra.h"
#include <Arduino.h>
#include <Wire.h>
#include <cmath>

namespace {
constexpr float kDegToRad = 0.01745329251994329577f;
}

Gyro::Gyro() = default;

void Gyro::begin() {
    Wire.begin();
    mpu.initialize();
    Serial.println(F("Testing MPU6050 connection..."));
    if (!mpu.testConnection()) {
        Serial.println(F("MPU6050 connection failed"));
    } else {
        Serial.println(F("MPU6050 connection successful"));
    }

    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    accelLsbPerG = 16384.0f; // ±2g

    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
    gyroLsbPerDPS = 16.4f; // ±2000 °/s

    algebra::Vector<3> gb { 0.0f, 0.0f, 0.0f };
    constexpr int samples = 500;
    for (int i = 0; i < samples; ++i) {
        int16_t ax, ay, az, gx, gy, gz;
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        gb[0] += (gx / gyroLsbPerDPS) * kDegToRad;
        gb[1] += (gy / gyroLsbPerDPS) * kDegToRad;
        gb[2] += (gz / gyroLsbPerDPS) * kDegToRad;
        delay(1);
    }
    gyroBias = algebra::Vector<3> { gb[0] / samples, gb[1] / samples,
                                    gb[2] / samples };

    Serial.print(F("Gyro bias (rad/s): "));
    Serial.print(gyroBias[0], 6);
    Serial.print(F(", "));
    Serial.print(gyroBias[1], 6);
    Serial.print(F(", "));
    Serial.println(gyroBias[2], 6);

    lastMicros = micros();
}

void Gyro::update() {
    unsigned long now = micros();
    float dt = (now - lastMicros) * 1e-6f;
    if (dt <= 0.0f || dt > 0.1f) dt = 0.001f;
    lastMicros = now;

    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);

    algebra::Vector<3> a_body_mps2 { (ax_raw / accelLsbPerG) * G0,
                                     (ay_raw / accelLsbPerG) * G0,
                                     (az_raw / accelLsbPerG) * G0 };
    const float gx_dps = gx_raw / gyroLsbPerDPS;
    const float gy_dps = gy_raw / gyroLsbPerDPS;
    const float gz_dps = gz_raw / gyroLsbPerDPS;
    algebra::Vector<3> g_body_rads { gx_dps * kDegToRad - gyroBias[0],
                                     gy_dps * kDegToRad - gyroBias[1],
                                     gz_dps * kDegToRad - gyroBias[2] };

    const float ax_n = ax_raw / accelLsbPerG;
    const float ay_n = ay_raw / accelLsbPerG;
    const float az_n = az_raw / accelLsbPerG;
    mahonyUpdate(g_body_rads[0], g_body_rads[1], g_body_rads[2], ax_n, ay_n,
                 az_n, dt);

    algebra::Vector<3> a_world_mps2 = algebra::rotateVector(q, a_body_mps2);
    linearAccelWorld = algebra::Vector<3> { a_world_mps2[0], a_world_mps2[1],
                                            a_world_mps2[2] - G0 };

    const float accelMag_g = std::sqrt(ax_n * ax_n + ay_n * ay_n + az_n * az_n);
    const float gyroMag_dps =
        std::sqrt(gx_dps * gx_dps + gy_dps * gy_dps + gz_dps * gz_dps);
    const bool isStill = (std::fabs(accelMag_g - 1.0f) < stillAccelThresh_g) &&
                         (gyroMag_dps < stillGyroThresh_dps);

    if (isStill) {
        velocity = algebra::Vector<3> { 0.0f, 0.0f, 0.0f };
    } else {
        velocity = algebra::Vector<3> { velocity[0] + linearAccelWorld[0] * dt,
                                        velocity[1] + linearAccelWorld[1] * dt,
                                        velocity[2] + linearAccelWorld[2] * dt };
        velocity = algebra::Vector<3> { velocity[0] * velDamping,
                                        velocity[1] * velDamping,
                                        velocity[2] * velDamping };
    }

    if (isStill && !wasStill) {
        Serial.println(F("Gyro: still detected -> velocity reset"));
    } else if (!isStill && wasStill) {
        Serial.println(F("Gyro: motion detected"));
    }
    wasStill = isStill;

    position = algebra::Vector<3> { position[0] + velocity[0] * dt,
                                    position[1] + velocity[1] * dt,
                                    position[2] + velocity[2] * dt };
}

algebra::Vector<3> Gyro::getLinearAcceleration() const {
    return linearAccelWorld;
}

algebra::Vector<3> Gyro::getOrientationEuler() const {
    return algebra::quaternionToEuler(q);
}

algebra::Vector<3> Gyro::getPosition() const { return position; }

algebra::Vector<3> Gyro::getVelocity() const { return velocity; }

void Gyro::mahonyUpdate(float gx_rad, float gy_rad, float gz_rad, float ax,
                        float ay, float az, float dt) {
    float norm = std::sqrt(ax * ax + ay * ay + az * az);
    if (norm > 1e-6f) {
        ax /= norm;
        ay /= norm;
        az /= norm;
    } else {
        ax = ay = az = 0.0f;
    }

    const float qw = q.w;
    const float qx = q.x;
    const float qy = q.y;
    const float qz = q.z;

    const float vx = 2.0f * (qx * qz - qw * qy);
    const float vy = 2.0f * (qw * qx + qy * qz);
    const float vz = qw * qw - qx * qx - qy * qy + qz * qz;

    const float ex = (ay * vz - az * vy);
    const float ey = (az * vx - ax * vz);
    const float ez = (ax * vy - ay * vx);

    if (twoKi > 0.0f) {
        integralFB[0] += twoKi * ex * dt;
        integralFB[1] += twoKi * ey * dt;
        integralFB[2] += twoKi * ez * dt;
    } else {
        integralFB = algebra::Vector<3> { 0.0f, 0.0f, 0.0f };
    }

    gx_rad += twoKp * ex + integralFB[0];
    gy_rad += twoKp * ey + integralFB[1];
    gz_rad += twoKp * ez + integralFB[2];

    const float qw_dot = 0.5f * (-qx * gx_rad - qy * gy_rad - qz * gz_rad);
    const float qx_dot = 0.5f * (qw * gx_rad + qy * gz_rad - qz * gy_rad);
    const float qy_dot = 0.5f * (qw * gy_rad + qz * gx_rad - qx * gz_rad);
    const float qz_dot = 0.5f * (qw * gz_rad + qx * gy_rad - qy * gx_rad);

    algebra::Quaternion next { qw + qw_dot * dt, qx + qx_dot * dt,
                               qy + qy_dot * dt, qz + qz_dot * dt };
    q = algebra::normalizeQuaternion(next);
}
