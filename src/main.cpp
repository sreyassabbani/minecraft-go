#include <Arduino.h>
#include <math.h>

#include <bno085.hpp>
#include <display.hpp>
#include <game_state.hpp>
#include <general.hpp>
#include <input_handler.hpp>
#include <renderer.hpp>

using ActiveImu = Bno085Imu;
using Vec3 = algebra::Vector<3>;

// Joystick wiring: X=A6, Y=A7, sprint button=digital 53 (active-low)
// Jump button: digital 6 (active-low)
// Block interactions: place on digital 7 (active-low), remove on digital 8
// (active-low)
constexpr int kJoyXPin = A6;
constexpr int kJoyYPin = A7;
constexpr int kJoyButtonPin = 53;
constexpr int kJumpButtonPin = 6;
constexpr int kPlaceButtonPin = 7;
constexpr int kRemoveButtonPin = 8;

constexpr float kBaseMove = 1.0f;         // base movement scale
constexpr float kSprintMultiplier = 2.0f; // sprint speed multiplier
constexpr int kJoyDeadzone = 50;          // ADC counts (~5%)
constexpr float kBlockReach = 4.0f;       // meters to reach for place/remove

// Core game objects
ActiveImu imu;
Player player(&imu);
display::Display* displayPtr = nullptr;
Renderer* rendererPtr = nullptr;
GameState* gamePtr = nullptr;
InputHandler* inputPtr = nullptr;

// High-level setup steps
static void initImu();
static void initPlayer();
static void initRenderAndGame();
static void initInput();
static void logImu(uint32_t nowMs);

void setup() {
    Serial.begin(115200);
    println("[Main] Booting...");

    initImu();
    initPlayer();
    initRenderAndGame();
    initInput();
}

void loop() {
    static uint32_t lastMs = millis();
    const uint32_t nowMs = millis();
    float dt = (nowMs - lastMs) / 1000.0f;
    lastMs = nowMs;
    if (dt > 0.1f) dt = 0.1f; // clamp large pauses

    imu.update();

    if (inputPtr && gamePtr) { inputPtr->update(player, *gamePtr, dt); }
    if (rendererPtr && gamePtr) { gamePtr->update(dt); }

    logImu(nowMs);
}

static void initImu() {
    println("[Main] Starting with BNO085 IMU");
    imu.begin();
    if (!imu.isInitialized()) {
        println("[Main] BNO085 not responding; using zero pose for now");
    }
}

static void initPlayer() {
    // Place player near world center at a safe height on the floor
    player.position = Vec3({ World::WIDTH * 0.5f, 2.2f, World::DEPTH * 0.5f });
    player.velocity = Vec3({ 0.0f, 0.0f, 0.0f });
}

static void initRenderAndGame() {
    displayPtr = &display::screen();
    println("[Main] Display dimensions:", displayPtr->width(), "x",
            displayPtr->height());
    rendererPtr = new Renderer(*displayPtr);
    gamePtr = new GameState(*rendererPtr, player);
    println("[Main] Spawned player at:", player.position[0], ",",
            player.position[1], ",", player.position[2]);

    pinMode(kJoyButtonPin, INPUT_PULLUP);
    pinMode(kJumpButtonPin, INPUT_PULLUP);
}

static void initInput() {
    InputPins pins { kJoyXPin,       kJoyYPin,        kJoyButtonPin,
                     kJumpButtonPin, kPlaceButtonPin, kRemoveButtonPin };
    inputPtr = new InputHandler(pins, kBaseMove, kSprintMultiplier,
                                kJoyDeadzone, kBlockReach);
    inputPtr->begin();
}

static void logImu(uint32_t nowMs) {
    if (!imu.isInitialized()) return;
    static uint32_t lastImuPrint = 0;
    if (nowMs - lastImuPrint < 200) return;

    const auto gravity = imu.getGravityVector();
    const auto accel = imu.getLinearAcceleration();
    const auto euler = imu.getOrientationEuler();
    println("[Main] gravity (m/s^2):", gravity[0], gravity[1], gravity[2]);
    println("[Main] linear accel (m/s^2):", accel[0], accel[1], accel[2]);
    println("[Main] yaw/pitch/roll (rad):", euler[0], euler[1], euler[2]);
    println("----");
    println("Player pos (m):", player.position[0], player.position[1],
            player.position[2]);
    lastImuPrint = nowMs;
}
