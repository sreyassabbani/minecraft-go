#include <bno085.hpp>

namespace {
constexpr uint32_t kReportIntervalUs = 10000; // 100 Hz
}

Bno085Imu::Bno085Imu() = default;

void Bno085Imu::begin() {
    Wire.begin();
    require(bno.begin_I2C(), "Failed to find BNO085 chip", "BNO085 Found!");

    configureReports();
    println("[BNO] Reports configured");
}

void Bno085Imu::configureReports() {
    // Prefer GAME_ROTATION_VECTOR (no magnetometer) to avoid heading drift
    bno.enableReport(SH2_GAME_ROTATION_VECTOR, kReportIntervalUs);
    bno.enableReport(SH2_LINEAR_ACCELERATION, kReportIntervalUs);
    bno.enableReport(SH2_GRAVITY, kReportIntervalUs);
}

void Bno085Imu::update() {
    // Consume all pending sensor events
    while (bno.getSensorEvent(&sensorValue)) {
        switch (sensorValue.sensorId) {
        case SH2_GAME_ROTATION_VECTOR: {
            const auto& r = sensorValue.un.gameRotationVector;
            q = algebra::normalizeQuaternion(
                algebra::Quaternion { r.real, r.i, r.j, r.k });
            break;
        }
        case SH2_LINEAR_ACCELERATION: {
            const auto& a = sensorValue.un.linearAcceleration;
            acceleration = algebra::Vector<3> { a.x, a.y, a.z };
            break;
        }
        case SH2_GRAVITY: {
            const auto& g = sensorValue.un.gravity;
            gravity = algebra::Vector<3> { g.x, g.y, g.z };
            break;
        }
        default: break;
        }
    }
}

algebra::Vector<3> Bno085Imu::getLinearAcceleration() const {
    return acceleration;
}

algebra::Vector<3> Bno085Imu::getOrientationEuler() const {
    return algebra::quaternionToEuler(q);
}

algebra::Quaternion
Bno085Imu::getOrientation(const algebra::Quaternion& current) const {
    (void)current;
    return q;
}

algebra::Vector<3> Bno085Imu::getPosition() const { return position; }

algebra::Vector<3> Bno085Imu::getVelocity() const { return velocity; }

algebra::Vector<3> Bno085Imu::getGravityVector() const { return gravity; }
