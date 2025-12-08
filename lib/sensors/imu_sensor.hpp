#pragma once

#include <algebra.hpp>

// Minimal interface for IMU/gyro implementations so game code stays hardware
// agnostic.
class Imu {
public:
    virtual ~Imu() = default;

    virtual void begin() = 0;
    virtual void update() = 0;

    virtual algebra::Vector<3> getLinearAcceleration() const = 0;
    virtual algebra::Vector<3> getOrientationEuler() const = 0;
    virtual algebra::Quaternion
    getOrientation(const algebra::Quaternion& current) const = 0;
    virtual algebra::Vector<3> getPosition() const = 0;
    virtual algebra::Vector<3> getVelocity() const = 0;
};
