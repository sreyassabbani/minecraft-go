#include <Arduino.h>
#include <algebra.hpp>
#include <general.hpp>
#include <gyro.hpp>

Gyro gyro;
unsigned long lastPrintMs = 0;

void setup() {
  Serial.begin(9600);
  gyro.begin();
}

void loop() {
  gyro.update();

  const unsigned long nowMs = millis();
  if (nowMs - lastPrintMs >= 500) {
    const auto pos = gyro.getPosition();
    const auto vel = gyro.getVelocity();
    const auto euler = gyro.getOrientationEuler();

    println("pos (m): ", pos[0], ", ", pos[1], ", ", pos[2],
            " | vel (m/s): ", vel[0], ", ", vel[1], ", ", vel[2],
            " | yaw/pitch/roll (rad): ", euler[0], ", ", euler[1], ", ", euler[2]);

    lastPrintMs = nowMs;
  }
}
