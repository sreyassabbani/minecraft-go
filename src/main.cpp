#include <Arduino.h>
#include <display.hpp>

using display::screen;

void drawSplash() {
    display::TextStyle title { display::Color::White(), display::Color::Black(),
                               3 };
    display::TextStyle subtitle { display::Color::Gray(),
                                  display::Color::Black(), 2 };

    screen().clear(display::Color::Black());
    screen().drawTextCentered(screen().width() / 2, screen().height() / 2 - 20,
                            "Display OK", title);
    screen().drawTextCentered(screen().width() / 2, screen().height() / 2 + 20,
                            "ST77xx Test", subtitle);
}

void drawColorBars() {
    const uint16_t barH = screen().height() / 6;
    display::Color bars[] = {
        display::Color::Red(),  display::Color::Green(),
        display::Color::Blue(), display::Color::White(),
        display::Color::Gray(), display::Color::fromRGB(255, 165, 0) // orange
    };

    for (uint8_t i = 0; i < 6; ++i) {
        screen().fillRect(0, i * barH, screen().width(), barH, bars[i]);
    }
}

void setup() {
    // baud rate for Due
    Serial.begin(115200);

    // initializes a static `display::Display`
    screen();

    drawSplash();
    delay(1500);
}

void loop() {
    drawColorBars();
    delay(1000);
    drawSplash();
    delay(1000);
}

// #include <Arduino.h>
// #include <algebra.hpp>
// #include <general.hpp>
// #include <gyro.hpp>

// Gyro gyro;
// unsigned long lastPrintMs = 0;

// static void printSigned(float value, uint8_t digits = 2) {
//     if (value >= 0.0f) Serial.print('+');
//     Serial.print(value, digits);
// }

// static void printVectorSigned(const algebra::Vector<3>& v, uint8_t digits =
// 2) {
//     printSigned(v[0], digits);
//     Serial.print(" ");
//     printSigned(v[1], digits);
//     Serial.print(" ");
//     printSigned(v[2], digits);
// }

// void setup() {
//     Serial.begin(9600);
//     println("Hello!");
//     gyro.begin();
// }

// void loop() {
//     // gyro.update();
//     // println("Updated!");
//     gyro.update();
//     const auto a = gyro.getLinearAcceleration();

//     // this is NOT correct. You need to do additional math to get the normal
//     vector to the camera const auto cam_normal = gyro.getOrientationEuler();

//     printVectorSigned(a);
//     Serial.println();
//     // println("UPDATED!");
//     // const unsigned long nowMs = millis();
//     // if (nowMs - lastPrintMs >= 500) {
//     //     const auto pos = gyro.getPosition();
//     //     const auto vel = gyro.getVelocity();
//     //     const auto euler = gyro.getOrientationEuler();

//     //     println("pos (m): ", pos[0], ", ", pos[1], ", ", pos[2],
//     //             " | vel (m/s): ", vel[0], ", ", vel[1], ", ", vel[2],
//     //             " | yaw/pitch/roll (rad): ", euler[0], ", ", euler[1], ",
//     ",
//     //             euler[2]);

//     //     lastPrintMs = nowMs;
//     // }
// }
