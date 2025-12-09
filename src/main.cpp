#include <Arduino.h>
#include <math.h>

#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <renderer.hpp>

#include <bno085.hpp>
using ActiveImu = Bno085Imu;

// Joystick wiring: X=A6, Y=A7, sprint button=digital 53 (active-low)
// Jump button: digital 6 (active-low)
constexpr int kJoyXPin = A6;
constexpr int kJoyYPin = A7;
constexpr int kJoyButtonPin = 53;
constexpr int kJumpButtonPin = 6;
constexpr int kJoyDeadzone = 50;          // ADC counts (~5%)
constexpr float kBaseMove = 1.0f;         // base movement scale
constexpr float kSprintMultiplier = 2.0f; // sprint speed multiplier

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

    if (!displayPtr) {
        displayPtr = &display::screen(); // initialize display hardware now
    }
    println("[Main] Display dimensions:", displayPtr->width(), "x",
            displayPtr->height());

    rendererPtr = new Renderer(*displayPtr);
    gamePtr = new GameState(*rendererPtr, player);

    // Place player near world center at a safe height on the floor
    player.position =
        algebra::Vector<3>({ World::WIDTH * 0.5f, 2.2f, World::DEPTH * 0.5f });
    player.velocity = algebra::Vector<3>({ 0.0f, 0.0f, 0.0f });
    println("[Main] Spawned player at:", player.position[0], ",",
            player.position[1], ",", player.position[2]);

    pinMode(kJoyButtonPin, INPUT_PULLUP);
    pinMode(kJumpButtonPin, INPUT_PULLUP);
}

void loop() {
    static uint32_t last = millis();
    const uint32_t now = millis();
    float dt = (now - last) / 1000.0f; // milliseconds -> seconds
    last = now;

    if (dt > 0.1f) dt = 0.1f; // clamp large pauses

    imu.update();

    // Joystick input
    const int rawX = analogRead(kJoyXPin);
    const int rawY = analogRead(kJoyYPin);
    const int sprintBtn = digitalRead(kJoyButtonPin);
    const int jumpBtn = digitalRead(kJumpButtonPin);

    auto normAxis = [](int raw) {
        // Map 0-1023 to roughly -1..+1 with a deadzone around center
        const int centered = raw - 512;
        if (abs(centered) < kJoyDeadzone) return 0.0f;
        return static_cast<float>(centered) / 512.0f;
    };

    const float joyX = normAxis(rawX);  // strafe
    const float joyY = -normAxis(rawY); // forward/back (invert so up=forward)

    float speed = kBaseMove;
    if (sprintBtn == LOW) { speed *= kSprintMultiplier; }

    if (fabsf(joyY) > 0.0f) { player.moveForward(joyY * speed * dt); }
    if (fabsf(joyX) > 0.0f) { player.moveRight(joyX * speed * dt); }

    static int lastJumpState = HIGH;
    if (lastJumpState == HIGH && jumpBtn == LOW) { player.jump(); }
    lastJumpState = jumpBtn;

    // Update physics and render via GameState every frame
    if (rendererPtr && gamePtr) { gamePtr->update(dt); }

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
    }
}
