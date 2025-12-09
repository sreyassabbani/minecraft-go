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

    // Optional hint for consumers that only want to override orientation when
    // the IMU has delivered a valid estimate.
    virtual bool hasOrientation() const { return true; }

    // Optional: recalibrate reference to the current pose.
    virtual void calibrateToCurrent() {}
};
