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

// Joystick wiring: X=A6, Y=A7
// Buttons (active-low): sprint=51, jump=49, place=47, remove=45
constexpr int kJoyXPin = A6;
constexpr int kJoyYPin = A7;
constexpr int kSprintButtonPin = 51;
constexpr int kJumpButtonPin = 49;
constexpr int kPlaceButtonPin = 47;
constexpr int kRemoveButtonPin = 45;

constexpr float kBaseMove = 2.0f;         // base movement scale (faster)
constexpr float kSprintMultiplier = 2.0f; // sprint speed multiplier
constexpr int kJoyDeadzone = 200;         // ADC counts (~20%) to reduce drift
constexpr float kBlockReach = 1.0f;       // interact with adjacent block only

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
static void drawStartMenu();
static void dismissStartMenu();

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

    // Show a simple start menu while IMU is calibrating/zeroing and keep it
    // up for at least 5 seconds total (even if IMU finishes early).
    static bool startMenuActive = true;
    static bool startMenuDrawn = false;
    static bool startMenuFinished = false;
    static uint32_t startMenuStartMs = 0;
    const uint32_t kMinMenuMs = 5000;

    const bool imuReady = imu.hasOrientation();
    if (!startMenuFinished && !startMenuDrawn && displayPtr) {
        drawStartMenu(); // draw once
        startMenuDrawn = true;
        startMenuStartMs = nowMs;
    }

    if (!imuReady) {
        startMenuActive = true;
        return; // wait until IMU ready
    }

    // IMU ready; keep menu up for the minimum duration
    if (startMenuActive && !startMenuFinished) {
        if (nowMs - startMenuStartMs >= kMinMenuMs) {
            dismissStartMenu();
            startMenuActive = false;
            startMenuDrawn = false;
            startMenuStartMs = 0;
            startMenuFinished = true;
        } else {
            return;
        }
    }

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
}

static void initInput() {
    InputPins pins { kJoyXPin,       kJoyYPin,        kSprintButtonPin,
                     kJumpButtonPin, kPlaceButtonPin, kRemoveButtonPin };

    // Ensure all button pins use internal pull-ups for stable reads.
    pinMode(kSprintButtonPin, INPUT_PULLUP);
    pinMode(kJumpButtonPin, INPUT_PULLUP);
    pinMode(kPlaceButtonPin, INPUT_PULLUP);
    pinMode(kRemoveButtonPin, INPUT_PULLUP);

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
    println("[Main] yaw/roll/pitch (rad):", euler[0], euler[2], euler[1]);
    println("[Main] btn 51/49/47/45:",
            digitalRead(kSprintButtonPin), ",",
            digitalRead(kJumpButtonPin), ",",
            digitalRead(kPlaceButtonPin), ",",
            digitalRead(kRemoveButtonPin));
    println("----");
    println("Player pos (m):", player.position[0], player.position[1],
            player.position[2]);
    lastImuPrint = nowMs;
}

static void drawStartMenu() {
    if (!displayPtr) return;
    displayPtr->clear(display::Color::Black());
    const int16_t cx = displayPtr->width() / 2;
    const int16_t cy = displayPtr->height() / 2;
    displayPtr->drawTextCentered(cx, cy - 36, "Minecraft Go",
                                 display::TextStyle(display::Color::Green(),
                                                    display::Color::Black(), 2));
    displayPtr->drawTextCentered(cx, cy - 12, "Calibrating IMU...",
                                 display::TextStyle(display::Color::White(),
                                                    display::Color::Black(), 1));
    displayPtr->drawTextCentered(cx, cy + 4, "Hold still",
                                 display::TextStyle(display::Color::Gray(),
                                                    display::Color::Black(), 1));

    // Controls
    const int16_t left = cx - 90;
    const int16_t top = cy + 24;
    displayPtr->drawText(left, top, "Controls:",
                         display::TextStyle(display::Color::White(),
                                            display::Color::Black(), 1));
    displayPtr->drawText(left, top + 12, "Btn 1 (Red): Destroy",
                         display::TextStyle(display::Color::Red(),
                                            display::Color::Black(), 1));
    displayPtr->drawText(left, top + 24, "Btn 2 (Blue): Place",
                         display::TextStyle(display::Color::Blue(),
                                            display::Color::Black(), 1));
    displayPtr->drawText(left, top + 36, "Btn 3 (Yellow): Jump",
                         display::TextStyle(display::Color::fromRGB(255, 215, 0),
                                            display::Color::Black(), 1));
    displayPtr->drawText(left, top + 52,
                         "Joystick: Up/Down=Forward/Back",
                         display::TextStyle(display::Color::White(),
                                            display::Color::Black(), 1));
    displayPtr->drawText(left, top + 64,
                         "          Left/Right=Strafe",
                         display::TextStyle(display::Color::White(),
                                            display::Color::Black(), 1));
}

static void dismissStartMenu() {
    if (!displayPtr) return;
    displayPtr->clear(display::Color::SkyBlue());
}
