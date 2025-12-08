#pragma once

#include <Adafruit_BNO08x.h>
#include <Arduino.h>
#include <Wire.h>
#include <algebra.hpp>
#include <general.hpp>
#include <imu_sensor.hpp>

// Minimal BNO085 wrapper for fused orientation + acceleration readings.
class Bno085Imu : public Imu {
public:
    Bno085Imu();

    void begin() override;
    void update() override;

    algebra::Vector<3> getLinearAcceleration() const override;
    algebra::Vector<3> getGravityVector() const;
    algebra::Vector<3> getOrientationEuler() const override;
    algebra::Quaternion
    getOrientation(const algebra::Quaternion& current) const override;
    algebra::Vector<3> getPosition() const override;
    algebra::Vector<3> getVelocity() const override;
    bool hasOrientation() const override {
        return initialized && eventCount > 0 && referenceSet;
    }
    bool isInitialized() const { return initialized; }

private:
    void configureReports();
    void scanI2cBus(TwoWire& bus, const char* busName);
    bool tryBegin(TwoWire& bus, uint8_t addr);

    Adafruit_BNO08x bno;
    TwoWire* wire = &Wire;
    sh2_SensorValue_t sensorValue {};
    bool initialized = false;
    uint32_t lastEventMs = 0;
    uint32_t lastNoEventLogMs = 0;
    uint32_t eventCount = 0;
    bool referenceSet = false;
    algebra::Quaternion reference { 1.0f, 0.0f, 0.0f, 0.0f };

    algebra::Quaternion q { 1.0f, 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> linearAccel { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> gravity { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> position { 0.0f, 0.0f, 0.0f };
    algebra::Vector<3> velocity { 0.0f, 0.0f, 0.0f };
};
