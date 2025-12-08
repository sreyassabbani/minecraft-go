#include <Arduino.h>
#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <renderer.hpp>

#include <bno085.hpp>
using ActiveImu = Bno085Imu;
// int joystickX = 0;
// int joystickY = 0;
// int buttonState = 0;

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

    if (!displayPtr)
        displayPtr = &display::screen(); // initialize display hardware now
    println("[Main] Display dimensions:", displayPtr->width(), "x",
            displayPtr->height());

    rendererPtr = new Renderer(*displayPtr);
    gamePtr = new GameState(*rendererPtr, player);

    // Place player near world center at a safe height on the floor
    player.position = algebra::Vector<3>({ 3.0f, 5.05f, 4.0f });
    player.velocity = algebra::Vector<3>({ 0.0f, 0.0f, 0.0f });
    println("[Main] Spawned player at:", player.position[0], ",",
            player.position[1], ",", player.position[2]);
    println("Display test complete");

    // pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
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
    if (imu.isInitialized() && now - lastImuPrint > 200) {
        const auto gravity = imu.getGravityVector();
        const auto accel = imu.getLinearAcceleration();
        const auto euler = imu.getOrientationEuler();
        println("[Main] gravity (m/s^2):", gravity[0], gravity[1], gravity[2]);
        println("[Main] linear accel (m/s^2):", accel[0], accel[1], accel[2]);
        println("[Main] yaw/pitch/roll (rad):", euler[0], euler[1], euler[2]);
        println("----");
        println("Player pos (m):", player.position[0], player.position[1],
                player.position[2]);
        lastImuPrint = now;
    // Disable physics; drive camera/player manually in an orbit
    game.update(dt);

    // Orbit around the world center while looking at it
    // const Vec3 center = Vec3({ World::WIDTH * 0.5f, 1.5f, World::DEPTH * 0.5f });
    // const float radius = 8.0f;     // circle around the 10x10 world
    // const float height = 3.0f;     // keep above the ground
    // const float angularSpeed = 0.4f; // radians per second
    // const float angle = angularSpeed * (millis() / 1000.0f);

    // game.player.position =
    //     Vec3({ center[0] + cosf(angle) * radius, height,
    //            center[2] + sinf(angle) * radius });
    // game.player.velocity = Vec3({ 0.0f, 0.0f, 0.0f });
    // game.player.orientation = algebra::lookAt(game.player.position, center);

    // Render full world (throttled for performance)
    static uint32_t lastRender = 0;
    if (now - lastRender > 100) { // Render every 100ms (10 FPS)
        if (renderer) {
            renderer->render(game.world, game.player.position,
                             game.player.orientation);
        }
        lastRender = now;
    }

    // Update physics and render via GameState
    if (rendererPtr && gamePtr) { gamePtr->update(dt); }
}
