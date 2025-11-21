#include <Arduino.h>
#include <Wire.h>
#include <algebra.hpp>
#include <general.hpp>
#include <gyro.hpp>
#if !defined(__AVR__) && !defined(ARDUINO_ARCH_AVR)
#include <cmath>
#endif

namespace {
constexpr float kDegToRad = 0.01745329251994329577f;
}

Gyro::Gyro() = default;

void Gyro::begin() {
    Wire.begin();
    require(mpu.begin(), "Failed to find MPU6050 chip", "MPU6050 Found!");
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    Serial.print("Accelerometer range set to: ");
    switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
        println("+-2G");
        break;
    case MPU6050_RANGE_4_G:
        println("+-4G");
        break;
    case MPU6050_RANGE_8_G:
        println("+-8G");
        break;
    case MPU6050_RANGE_16_G:
        println("+-16G");
        break;
    }
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    Serial.print("Gyro range set to: ");
    switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
        println("+- 250 deg/s");
        break;
    case MPU6050_RANGE_500_DEG:
        println("+- 500 deg/s");
        break;
    case MPU6050_RANGE_1000_DEG:
        println("+- 1000 deg/s");
        break;
    case MPU6050_RANGE_2000_DEG:
        println("+- 2000 deg/s");
        break;
    }

    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.print("Filter bandwidth set to: ");
    switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
        println("260 Hz");
        break;
    case MPU6050_BAND_184_HZ:
        println("184 Hz");
        break;
    case MPU6050_BAND_94_HZ:
        println("94 Hz");
        break;
    case MPU6050_BAND_44_HZ:
        println("44 Hz");
        break;
    case MPU6050_BAND_21_HZ:
        println("21 Hz");
        break;
    case MPU6050_BAND_10_HZ:
        println("10 Hz");
        break;
    case MPU6050_BAND_5_HZ:
        println("5 Hz");
        break;
    }
}

void Gyro::update() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    acceleration[0] = a.acceleration.x;
    acceleration[1] = a.acceleration.y;
    acceleration[2] = a.acceleration.z;

    // do something with g soon
}

algebra::Vector<3> Gyro::getLinearAcceleration() const { return acceleration; }

algebra::Vector<3> Gyro::getOrientationEuler() const {
    return algebra::quaternionToEuler(q);
}

algebra::Vector<3> Gyro::getPosition() const { return position; }

algebra::Vector<3> Gyro::getVelocity() const { return velocity; }

void Gyro::mahonyUpdate(float gx_rad, float gy_rad, float gz_rad, float ax,
                        float ay, float az, float dt) {
#if defined(__AVR__) || defined(ARDUINO_ARCH_AVR)
    (void)gx_rad;
    (void)gy_rad;
    (void)gz_rad;
    (void)ax;
    (void)ay;
    (void)az;
    (void)dt;
    // No-op on Uno to avoid math overhead; orientation stays identity.
#else
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
#endif
}
