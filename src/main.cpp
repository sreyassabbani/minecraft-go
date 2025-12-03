#include <Arduino.h>
#include <game_state.hpp>
#include <general.hpp>
#include <math.h>
#include <renderer.hpp>

using Vec3 = algebra::Vector<3>;
using display::screen;

GameState game;
// Renderer will be initialized in setup or globally if display is ready.
// Display is a singleton via screen().
Renderer* renderer = nullptr;

// init renderer here? or have game.render() ?

// init renderer here? or have game.render() ?

void setup() {
    Serial.begin(115200);
    // Initialize Display

    renderer = new Renderer(screen());

    println("Minecraft Go Initialized");

    // Test display by drawing a rectangle
    // println("Testing display...");
    // screen().fillRect(100, 100, 100, 100, display::Color::Red());
    // delay(2000); // Show red square for 2 seconds

    println("Display test complete");
}

void loop() {
    static uint32_t last = millis();
    uint32_t now = millis();
    float dt = (now - last) / 1000.0f; // milliseconds -> seconds
    last = now;

    if (dt > 0.1f) dt = 0.1f; // clamp large pauses

    // Disable physics; drive camera/player manually in an orbit
    // game.update(dt);

    // Orbit around the world center while looking at it
    const Vec3 center = Vec3({ World::WIDTH * 0.5f, 1.5f, World::DEPTH * 0.5f });
    const float radius = 8.0f;     // circle around the 10x10 world
    const float height = 3.0f;     // keep above the ground
    const float angularSpeed = 0.4f; // radians per second
    const float angle = angularSpeed * (millis() / 1000.0f);

    game.player.position =
        Vec3({ center[0] + cosf(angle) * radius, height,
               center[2] + sinf(angle) * radius });
    game.player.velocity = Vec3({ 0.0f, 0.0f, 0.0f });
    game.player.orientation = algebra::lookAt(game.player.position, center);

    // Render full world (throttled for performance)
    static uint32_t lastRender = 0;
    if (now - lastRender > 100) { // Render every 100ms (10 FPS)
        if (renderer) {
            renderer->render(game.world, game.player.position,
                             game.player.orientation);
        }
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
