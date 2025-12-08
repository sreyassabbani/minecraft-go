#pragma once

#include <Arduino.h>
#include <algebra.hpp>
#include <imu_sensor.hpp>
#include <world.hpp>

// Simple IMU stub that generates a smooth orbit around the world center so
// rendering and player logic can be exercised without hardware.
class DemoImu : public Imu {
public:
    DemoImu();

    void begin() override;
    void update() override;

    algebra::Vector<3> getLinearAcceleration() const override;
    algebra::Vector<3> getOrientationEuler() const override;
    algebra::Quaternion
    getOrientation(const algebra::Quaternion& current) const override;
    algebra::Vector<3> getPosition() const override;
    algebra::Vector<3> getVelocity() const override;

private:
    algebra::Quaternion orientation { 1.0f, 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> position { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> velocity { 0.0f, 0.0f, 0.0f };
    unsigned long startMillis = 0;
};
