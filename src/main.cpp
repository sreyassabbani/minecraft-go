#include <Arduino.h>
#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <renderer.hpp>

// Uncomment exactly one of these to choose the active IMU implementation.
#define USE_DEMO_IMU
// #define USE_BNO085

#ifdef USE_DEMO_IMU
#include <demo.hpp>
using ActiveImu = DemoImu;
#elif defined(USE_BNO085)
#include <bno085.hpp>
using ActiveImu = Bno085Imu;
#else
#include <mpu6050.hpp>
using ActiveImu = Mpu6050Imu;
#endif

using Vec3 = algebra::Vector<3>;

ActiveImu imu;
Player player(&imu);
Renderer renderer(display::screen());

GameState game(renderer, player);

void runDisplaySelfTest() {
    auto& disp = display::screen();
    println("[Main] Display self-test: clearing to red/green/blue");
    disp.clear(display::Color::Red());
    delay(200);
    disp.clear(display::Color::Green());
    delay(200);
    disp.clear(display::Color::Blue());
    delay(200);
    disp.clear(display::Color::Black());

    const char* msg = "Minecraft Go";
    disp.drawTextCentered(disp.width() / 2, disp.height() / 2, msg,
                          display::TextStyle(display::Color::White(),
                                             display::Color::Black(), 2));
    println("[Main] Display self-test done (text drawn)");
}

void setup() {
    Serial.begin(115200);

    imu.begin();
#ifdef USE_DEMO_IMU
    println("[Main] Running with DemoImu (orbiting pose)");
#elif defined(USE_BNO085)
    println("[Main] Running with BNO085 (sensor fusion)");
#else
    println("[Main] Running with MPU6050");
#endif
    println("Minecraft Go Initialized");

    println("[Main] Display dimensions:", display::screen().width(), "x",
            display::screen().height());
    runDisplaySelfTest();
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
#elif defined(USE_BNO085)
    // Periodic IMU debug (BNO085)
    static uint32_t lastImuPrint = 0;
    if (now - lastImuPrint > 500) {
        const auto gravity = imu.getGravityVector();
        const auto euler = imu.getOrientationEuler();
        Serial.print("[Main] BNO a (m/s^2): ");
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
        println("[Main] Rendered frame; player pos:", game.player.position[0],
                ",", game.player.position[1], ",", game.player.position[2]);
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
