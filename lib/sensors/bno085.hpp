#pragma once

#include <Adafruit_BNO08x.h>
#include <Arduino.h>
#include <algebra.hpp>
#include <general.hpp>
#include <imu_sensor.hpp>

class Bno085Imu : public Imu {
public:
    Bno085Imu();

    void begin() override;
    void update() override;

    algebra::Vector<3> getLinearAcceleration() const override;
    algebra::Vector<3> getOrientationEuler() const override;
    algebra::Quaternion
    getOrientation(const algebra::Quaternion& current) const override;
    algebra::Vector<3> getPosition() const override;
    algebra::Vector<3> getVelocity() const override;

    algebra::Vector<3> getGravityVector() const;

private:
    void configureReports();

    Adafruit_BNO08x bno;
    sh2_SensorValue_t sensorValue {};

    algebra::Quaternion q { 1.0f, 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> acceleration { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> gravity { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> position { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> velocity { 0.0f, 0.0f, 0.0f };
};
