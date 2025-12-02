#include <Arduino.h>
#include <game_state.hpp>
#include <renderer.hpp>

using Vec3 = algebra::Vector<3>;
using display::screen;

GameState game;
// Renderer will be initialized in setup or globally if display is ready.
// Display is a singleton via screen().
Renderer* renderer = nullptr;

// init renderer here? or have game.render() ?

void setup() {
    Serial.begin(115200);
    // Initialize Display

    renderer = new Renderer(screen());

    Serial.println("Minecraft Go Initialized");

    // Test display by drawing a rectangle
    Serial.println("Testing display...");
    screen().fillRect(100, 100, 100, 100, display::Color::Red());
    delay(2000); // Show red square for 2 seconds

    Serial.println("Display test complete");
}

void loop() {
    static uint32_t last = millis();
    uint32_t now = millis();
    float dt = (now - last) / 10000.0f;
    last = now;

    if (dt > 0.1f) dt = 0.01f;

    // Disable physics for now - freeze player at good viewing position
    // game.update(dt);

    // Set player to fixed position where they can see the world
    static bool playerPositioned = false;
    if (!playerPositioned) {
        // Camera at origin (0,0,0)
        // We are moving the world to be in front of the camera
        game.player.position = Vec3({ 0.0f, -1.0f, 0.0f });

        game.player.velocity = Vec3({ 0.0f, 0.0f, 0.0f });
        playerPositioned = true;

        Serial.println("Camera at (0, 0, 0) - world moved to be in front");
    }

    // Render full world (throttled for performance)
    static uint32_t lastRender = 0;
    if (now - lastRender > 100) { // Render every 100ms (10 FPS)
        if (renderer) { renderer->render(game.world, game.player); }
        lastRender = now;
    }

    game.update(dt);

    // Debug output
    static uint32_t lastPrint = 0;
    if (now - lastPrint > 1000) {
        Serial.print("Player Pos: ");
        Serial.print(game.player.position[0]);
        Serial.print(", ");
        Serial.print(game.player.position[1]);
        Serial.print(", ");
        Serial.println(game.player.position[2]);

        // FPS calculation could go here
        Serial.print("FPS: ");
        Serial.println(1.0f / dt);

        lastPrint = now;
    }

    // update camera normal based on IMU (depends on BNO055)
    // do not read position from IMU

    // get joystick (x, y, z) -> remove y to project to get direction of
    // movement. change player position by delta (x, 0, z).

    // game rules
}
