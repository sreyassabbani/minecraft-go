#include <Arduino.h>
#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <renderer.hpp>

// #define USE_DEMO_IMU

#ifdef USE_DEMO_IMU
#include <demo.hpp>
using ActiveImu = DemoImu;
#else
#include <mpu6050.hpp>
using ActiveImu = Mpu6050Imu;
#endif

using Vec3 = algebra::Vector<3>;

ActiveImu imu;
Player player(&imu);
Renderer renderer(display::screen());

GameState game(renderer, player);

void setup() {
    Serial.begin(115200);

    imu.begin();
#ifdef USE_DEMO_IMU
    println("[Main] Running with DemoImu (orbiting pose)");
#else
    println("[Main] Running with MPU6050");
#endif
    println("Minecraft Go Initialized");
}

void loop() {
    static uint32_t last = millis();
    uint32_t now = millis();
    float dt = (now - last) / 1000.0f; // milliseconds -> seconds
    last = now;

    if (dt > 0.1f) dt = 0.1f; // clamp large pauses

    imu.update();

#ifdef USE_DEMO_IMU
    // Drive the player pose from the demo IMU for hardware-free testing
    game.player.position = imu.getPosition();
    game.player.velocity = imu.getVelocity();
#else
    // Periodic IMU debug (real hardware)
    static uint32_t lastImuPrint = 0;
    if (now - lastImuPrint > 500) {
        const auto gravity = imu.getGravityVector();
        const auto euler = imu.getOrientationEuler();
        Serial.print("[Main] a (m/s^2): ");
        Serial.print(gravity[0]);
        Serial.print(", ");
        Serial.print(gravity[1]);
        Serial.print(", ");
        Serial.print(gravity[2]);
        Serial.print(" | yaw/pitch/roll (rad): ");
        Serial.print(euler[0]);
        Serial.print(", ");
        Serial.print(euler[1]);
        Serial.print(", ");
        Serial.println(euler[2]);
        lastImuPrint = now;
    }
#endif

    // Render full world (throttled for performance)
    static uint32_t lastRender = 0;
    if (now - lastRender > 100) { // Render every 100ms (10 FPS)
        game.render();
        lastRender = now;
    }

    // Debug output
    static uint32_t lastPrint = 0;
    if (now - lastPrint > 1000) {
        println("Player Pos:", game.player.position[0], ",",
                game.player.position[1], ",", game.player.position[2]);

        // FPS calculation could go here
        println("FPS:", 1.0f / dt);

        lastPrint = now;
    }

    // update camera normal based on IMU (depends on BNO085)
    // do not read position from IMU

    // get joystick (x, y, z) -> remove y to project to get direction of
    // movement. change player position by delta (x, 0, z).

    // game rules
}
