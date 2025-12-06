#include <Arduino.h>
#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <renderer.hpp>

#include <bno085.hpp>

using ActiveImu = Bno085Imu;
using Vec3 = algebra::Vector<3>;
using display::screen;

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

    if (!displayPtr) displayPtr = &display::screen(); // init display hardware
    println("[Main] Display dimensions:", displayPtr->width(), "x",
            displayPtr->height());

    rendererPtr = new Renderer(*displayPtr);
    gamePtr = new GameState(*rendererPtr, player);

    // Place player near world center at a safe height on the floor
    player.position = Vec3({ 3.0f, 5.05f, 4.0f });
    player.velocity = Vec3({ 0.0f, 0.0f, 0.0f });
    println("[Main] Spawned player at:", player.position[0], ",",
            player.position[1], ",", player.position[2]);
}

void loop() {
    static uint32_t last = millis();
    const uint32_t now = millis();
    float dt = (now - last) / 1000.0f; // milliseconds -> seconds
    last = now;

    if (dt > 0.1f) dt = 0.1f; // clamp large pauses

    imu.update();

    // Periodic IMU debug
    static uint32_t lastImuPrint = 0;
    if (imu.isInitialized() && now - lastImuPrint > 200) {
        const auto gravity = imu.getGravityVector();
        const auto accel = imu.getLinearAcceleration();
        const auto euler = imu.getOrientationEuler();
        println("[Main] gravity (m/s^2):", gravity[0], gravity[1], gravity[2]);
        println("[Main] linear accel (m/s^2):", accel[0], accel[1], accel[2]);
        println("[Main] yaw/roll/pitch (rad):", euler[0], euler[2], euler[1]);
        println("----");
        println("Player pos (m):", player.position[0], player.position[1],
                player.position[2]);
        lastImuPrint = now;
    }

    // Update physics and render via GameState
    if (rendererPtr && gamePtr) { gamePtr->update(dt); }
}
