#include <Arduino.h>
#include <Wire.h>
#include <algebra.hpp>
#include <cmath>
#include <general.hpp>
#include <mpu6050.hpp>

namespace {
algebra::Quaternion makeQuaternion(float yaw, float pitch, float roll) {
    const float halfYaw = yaw * 0.5f;
    const float halfPitch = pitch * 0.5f;
    const float halfRoll = roll * 0.5f;

    const float cy = std::cos(halfYaw);
    const float sy = std::sin(halfYaw);
    const float cp = std::cos(halfPitch);
    const float sp = std::sin(halfPitch);
    const float cr = std::cos(halfRoll);
    const float sr = std::sin(halfRoll);

    return algebra::normalizeQuaternion(algebra::Quaternion {
        cr * cp * cy + sr * sp * sy, // w
        sr * cp * cy - cr * sp * sy, // x
        cr * sp * cy + sr * cp * sy, // y
        cr * cp * sy - sr * sp * cy  // z
    });
}
} // namespace

Mpu6050Imu::Mpu6050Imu() = default;

void Mpu6050Imu::begin() {
    Wire.begin();
    require(mpu.begin(), "Failed to find MPU6050 chip", "MPU6050 Found!");
    println("[MPU] init complete, configuring ranges");

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

void Mpu6050Imu::update() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    static uint32_t sampleCount = 0;
    ++sampleCount;

    gravity = algebra::Vector<3> { a.acceleration.x, a.acceleration.y,
                                   a.acceleration.z };
    acceleration = gravity; // currently includes gravity; refine when needed

    const float magSq = gravity[0] * gravity[0] + gravity[1] * gravity[1] +
                        gravity[2] * gravity[2];
    if (magSq < 1e-6f) { return; }

    const float invMag = 1.0f / std::sqrt(magSq);
    const float ax = gravity[0] * invMag;
    const float ay = gravity[1] * invMag;
    const float az = gravity[2] * invMag;

    const float roll = std::atan2(ay, az);
    const float pitch = std::atan2(-ax, std::sqrt(ay * ay + az * az));

    const float yaw = algebra::quaternionToEuler(q)[0];
    q = makeQuaternion(yaw, pitch, roll);

    // Periodic debug
    if ((sampleCount % 20) == 0) {
        Serial.print("[MPU] a (m/s^2): ");
        Serial.print(gravity[0]);
        Serial.print(", ");
        Serial.print(gravity[1]);
        Serial.print(", ");
        Serial.print(gravity[2]);

        Serial.print(" | g (deg/s): ");
        Serial.print(g.gyro.x);
        Serial.print(", ");
        Serial.print(g.gyro.y);
        Serial.print(", ");
        Serial.print(g.gyro.z);

        Serial.print(" | roll/pitch (rad): ");
        Serial.print(roll);
        Serial.print(", ");
        Serial.print(pitch);

        Serial.print(" | yaw (rad): ");
        Serial.println(yaw);
    }
}

algebra::Vector<3> Mpu6050Imu::getLinearAcceleration() const {
    return acceleration;
}

algebra::Vector<3> Mpu6050Imu::getOrientationEuler() const {
    return algebra::quaternionToEuler(q);
}

Quaternion Mpu6050Imu::getOrientation(const algebra::Quaternion&) const {
    return q;
}

algebra::Vector<3> Mpu6050Imu::getGravityVector() const { return gravity; }

algebra::Vector<3> Mpu6050Imu::getPosition() const { return position; }

algebra::Vector<3> Mpu6050Imu::getVelocity() const { return velocity; }
