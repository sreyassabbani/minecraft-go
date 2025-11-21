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

inline algebra::Vector<3> rotateToWorld(const algebra::Quaternion& q,
                                        const algebra::Vector<3>& v_body) {
    return algebra::rotateVector(q, v_body);
}

inline void integrateEuler(algebra::Vector<3>& state,
                           const algebra::Vector<3>& deriv, float dt) {
    state[0] += deriv[0] * dt;
    state[1] += deriv[1] * dt;
    state[2] += deriv[2] * dt;
}
} // namespace

Gyro::Gyro() = default;

void Gyro::begin() {
    Wire.begin();
    require(mpu.begin(), "Failed to find MPU6050 chip", "MPU6050 Found!");

    // set range for accelerometer
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

    // set range for gyroscope
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

    // set low-pass filter bandwidth (cut high-frequency noise before reading)
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

    const unsigned long now = micros();
    if (lastMicros == 0) {
        lastMicros = now;
        acceleration = algebra::Vector<3> { 0.0f, 0.0f, 0.0f };
        velocity = algebra::Vector<3> { 0.0f, 0.0f, 0.0f };
        position = algebra::Vector<3> { 0.0f, 0.0f, 0.0f };
        return;
    }

    const float dt = (now - lastMicros) * 1.0e-6f;
    lastMicros = now;
    if (dt <= 0.0f) return;

    // Update orientation estimate (no-op on AVR)
    mahonyUpdate(g.gyro.x, g.gyro.y, g.gyro.z, a.acceleration.x,
                 a.acceleration.y, a.acceleration.z, dt);

    algebra::Vector<3> accelBody { a.acceleration.x, a.acceleration.y,
                                   a.acceleration.z };
    algebra::Vector<3> accelWorld = rotateToWorld(q, accelBody);
    accelWorld[2] -= G0; // remove gravity (world Z assumed up)

    acceleration = accelWorld;

    integrateEuler(velocity, acceleration, dt);
    velocity[0] *= velDamping;
    velocity[1] *= velDamping;
    velocity[2] *= velDamping;
    integrateEuler(position, velocity, dt);

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
