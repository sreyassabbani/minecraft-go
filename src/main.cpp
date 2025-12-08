#include <Arduino.h>
#include <bno085.hpp>
#include <general.hpp>

// Minimal BNO085 bring-up: read fused orientation and accel, print to serial.

Bno085Imu imu;

void setup() {
    Serial.begin(115200);
    println("[Main] BNO085 bring-up (println-only)");

    imu.begin();
    println("[Main] BNO085 init done");
}

void loop() {
    static uint32_t lastPrint = 0;
    imu.update();

    uint32_t now = millis();
    if (now - lastPrint >= 100) {
        const auto gravity = imu.getGravityVector();
        const auto accel = imu.getLinearAcceleration();
        const auto euler = imu.getOrientationEuler();

        println("[Main] gravity (m/s^2):", gravity[0], gravity[1], gravity[2]);
        println("[Main] linear accel (m/s^2):", accel[0], accel[1], accel[2]);
        println("[Main] yaw/pitch/roll (rad):", euler[0], euler[1], euler[2]);
        println("----");

        lastPrint = now;
    }
}
