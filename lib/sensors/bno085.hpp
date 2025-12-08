#pragma once

#include <Adafruit_BNO08x.h>
#include <Arduino.h>
#include <algebra.hpp>
#include <general.hpp>

// Minimal BNO085 wrapper for fused orientation + acceleration readings.
class Bno085Imu {
public:
    Bno085Imu();

    void begin();
    void update();

    algebra::Vector<3> getLinearAcceleration() const;
    algebra::Vector<3> getGravityVector() const;
    algebra::Vector<3> getOrientationEuler() const;
    algebra::Quaternion
    getOrientation(const algebra::Quaternion& current) const;

private:
    void configureReports();

    Adafruit_BNO08x bno;
    sh2_SensorValue_t sensorValue {};
    bool initialized = false;
    uint32_t lastEventMs = 0;
    uint32_t lastNoEventLogMs = 0;
    uint32_t eventCount = 0;

    algebra::Quaternion q { 1.0f, 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> linearAccel { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> gravity { 0.0f, 0.0f, 0.0f };
};
