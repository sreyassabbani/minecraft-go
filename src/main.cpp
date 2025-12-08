#include <Arduino.h>
#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <renderer.hpp>

#include <bno085.hpp>
using ActiveImu = Bno085Imu;

ActiveImu imu;
Player player(&imu);

display::Display* displayPtr = nullptr;
Renderer* rendererPtr = nullptr;
GameState* gamePtr = nullptr;

void setup() {
    Serial.begin(115200);
    println("[Main] Starting with BNO085 IMU");

    imu.begin();

    if (!imu.isInitialized()) {
        println("[Main] BNO085 not responding; skipping IMU prints and using "
                "zero pose");
    }

    if (!displayPtr)
        displayPtr = &display::screen(); // initialize display hardware now
    println("[Main] Display dimensions:", displayPtr->width(), "x",
            displayPtr->height());

    rendererPtr = new Renderer(*displayPtr);
    gamePtr = new GameState(*rendererPtr, player);
}

void loop() {
    static uint32_t last = millis();
    uint32_t now = millis();
    float dt = (now - last) / 1000.0f; // milliseconds -> seconds
    last = now;

    if (dt > 0.1f) dt = 0.1f; // clamp large pauses

    imu.update();

    // Periodic IMU debug
    static uint32_t lastImuPrint = 0;
    if (imu.isInitialized() && now - lastImuPrint > 100) {
        const auto gravity = imu.getGravityVector();
        const auto accel = imu.getLinearAcceleration();
        const auto euler = imu.getOrientationEuler();
        println("[Main] gravity (m/s^2):", gravity[0], gravity[1], gravity[2]);
        println("[Main] linear accel (m/s^2):", accel[0], accel[1], accel[2]);
        println("[Main] yaw/pitch/roll (rad):", euler[0], euler[1], euler[2]);
        println("----");
        lastImuPrint = now;
    }

    // Drive demo pose from IMU data (keeps renderer exercised)
    player.position = imu.getPosition();
    player.velocity = imu.getVelocity();

    static uint32_t lastRender = 0;
    if (rendererPtr && gamePtr &&
        now - lastRender > 50) { // Render every 100ms (10 FPS)
        gamePtr->render();
        lastRender = now;
    }
}
