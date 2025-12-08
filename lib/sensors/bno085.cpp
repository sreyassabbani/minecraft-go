#include <Wire.h>
#include <bno085.hpp>

namespace {
constexpr uint32_t kReportIntervalUs = 10000; // 100 Hz
}

Bno085Imu::Bno085Imu() = default;

bool Bno085Imu::tryBegin(TwoWire& bus, uint8_t addr) {
    bus.setClock(400000);
    println("[BNO] Probing bus at 0x", addr);
    return bno.begin_I2C(addr, &bus);
}

void Bno085Imu::scanI2cBus(TwoWire& bus, const char* busName) {
    println("[BNO] Scanning I2C bus ", busName, "...");
    for (uint8_t addr = 1; addr < 127; ++addr) {
        bus.beginTransmission(addr);
        const uint8_t err = bus.endTransmission();
        if (err == 0) {
            println("[BNO] Found device at 0x", addr, " (", (int)addr, ")");
        }
    }
}

void Bno085Imu::begin() {
    Wire.begin();
    Wire1.begin();
    Wire.setClock(400000);
    Wire1.setClock(400000);
    delay(50); // give sensor time to boot

    scanI2cBus(Wire, "Wire");
    scanI2cBus(Wire1, "Wire1");

    const uint8_t addrPrimary = 0x4A;
    const uint8_t addrAlt = 0x4B;

    println("[BNO] Probing Wire at 0x", addrPrimary, " then 0x", addrAlt);

    initialized = bno.begin_I2C(addrPrimary, &Wire);
    if (!initialized) {
        println("[BNO] Addr 0x", addrPrimary,
                " not responding; trying alt address 0x", addrAlt);
        initialized = bno.begin_I2C(addrAlt, &Wire);
    }

    if (!initialized) {
        println("[BNO] Trying Wire1 at 0x", addrPrimary, " then 0x", addrAlt);
        initialized = bno.begin_I2C(addrPrimary, &Wire1);
        if (!initialized) {
            println("[BNO] Addr 0x", addrPrimary,
                    " not responding; trying alt address 0x", addrAlt);
            initialized = bno.begin_I2C(addrAlt, &Wire1);
        }
    }

    if (!initialized) {
        println("[BNO] Neither address responded on either bus; check wiring/ADR pin/3V3 power");
        return;
    }

    configureReports();
    println("[BNO] Reports configured");
}

void Bno085Imu::configureReports() {
    // Prefer game rotation vector (no magnetometer) for stable heading
    require(bno.enableReport(SH2_GAME_ROTATION_VECTOR, kReportIntervalUs),
            "BNO: failed to enable GAME_ROTATION_VECTOR");
    require(bno.enableReport(SH2_LINEAR_ACCELERATION, kReportIntervalUs),
            "BNO: failed to enable LINEAR_ACCELERATION");
    require(bno.enableReport(SH2_GRAVITY, kReportIntervalUs),
            "BNO: failed to enable GRAVITY");

    lastEventMs = millis();
    lastNoEventLogMs = 0;
    eventCount = 0;
}

void Bno085Imu::update() {
    if (!initialized) return;

    if (bno.wasReset()) {
        println("[BNO] Detected reset, reconfiguring reports");
        configureReports();
    }

    bool gotEvent = false;
    while (bno.getSensorEvent(&sensorValue)) {
        gotEvent = true;
        lastEventMs = millis();
        ++eventCount;
        switch (sensorValue.sensorId) {
        case SH2_GAME_ROTATION_VECTOR: {
            const auto& r = sensorValue.un.gameRotationVector;
            q = algebra::normalizeQuaternion(
                algebra::Quaternion { r.real, r.i, r.j, r.k });
            break;
        }
        case SH2_LINEAR_ACCELERATION: {
            const auto& a = sensorValue.un.linearAcceleration;
            linearAccel = algebra::Vector<3> { a.x, a.y, a.z };
            break;
        }
        case SH2_GRAVITY: {
            const auto& g = sensorValue.un.gravity;
            gravity = algebra::Vector<3> { g.x, g.y, g.z };
            break;
        }
        default:
            break;
        }

        if (eventCount <= 5) {
            println("[BNO] Event ", sensorValue.sensorId,
                    " | q:", q.w, q.x, q.y, q.z, " | g:", gravity[0],
                    gravity[1], gravity[2], " | la:", linearAccel[0],
                    linearAccel[1], linearAccel[2]);
        }
    }

    if (!gotEvent) {
        const uint32_t now = millis();
        if (lastEventMs == 0) lastEventMs = now;
        if (now - lastEventMs > 500 && now - lastNoEventLogMs > 1000) {
            println("[BNO] No sensor events for ", (now - lastEventMs), " ms");
            lastNoEventLogMs = now;
        }
    }
}

algebra::Vector<3> Bno085Imu::getLinearAcceleration() const {
    return linearAccel;
}

algebra::Vector<3> Bno085Imu::getGravityVector() const { return gravity; }

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
