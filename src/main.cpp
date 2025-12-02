#include <Arduino.h>
#include "../lib/game/GameState.h"
#include "../lib/game/Renderer.h"
#include "../lib/game/Raster.h"
#include "../lib/display/display.hpp"
#include "../lib/algebra/algebra.hpp"

using Vec3 = algebra::Vector<3>;

GameState game;
// Renderer will be initialized in setup or globally if display is ready.
// Display is a singleton via display::screen().
Renderer* renderer = nullptr;

void setup() {
    Serial.begin(115200);
    // Initialize Display
    // Assuming display::screen() constructor handles initialization or we need to call something?
    // Looking at display.hpp, constructor calls tft().begin().
    // So accessing display::screen() should be enough.
    
    renderer = new Renderer(display::screen());
    
    Serial.println("Minecraft Go Initialized");
    
    // Test display by drawing a rectangle
    Serial.println("Testing display...");
    display::screen().fillRect(100, 100, 100, 100, display::Color::Red());
    delay(2000); // Show red square for 2 seconds
    
    Serial.println("Display test complete");
}

void loop() {
    static uint32_t last = millis();
    uint32_t now = millis();
    float dt = (now - last) / 1000.0f;
    last = now;

    if (dt > 0.1f) dt = 0.1f;

    // Disable physics for now - freeze player at good viewing position
    // game.update(dt);
    
    // Set player to fixed position where they can see the world
    static bool playerPositioned = false;
    if (!playerPositioned) {
        // X=5 centers on world (world is 0-10)
        // Y=0 works correctly
        // Z=-8 to see depth
        game.player.position = Vec3({5.0f, 0.0f, -8.0f});
        
        game.player.velocity = Vec3({0.0f, 0.0f, 0.0f});
        playerPositioned = true;
        
        Serial.println("Camera at (5, 0, -8) - centered, ground level");
    }
    
    // Render full world (throttled for performance)
    static uint32_t lastRender = 0;
    if (now - lastRender > 100) { // Render every 100ms (10 FPS)
        if (renderer) {
            renderer->render(game.world, game.player);
        }
        lastRender = now;
    }
    
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
}
