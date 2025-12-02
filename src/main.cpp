#include "../lib/game/GameState.h"
#include <Arduino.h>

GameState game;

// init renderer here? or have game.render() ?

void setup() {
    Serial.begin(115200);
    // Initialize other HAL components here if needed (Display, Sensors)
}

void loop() {
    static uint32_t last = millis();
    uint32_t now = millis();
    float dt = (now - last) / 1000.0f;
    last = now;

    // Clamp dt to avoid huge jumps if loop hangs
    if (dt > 0.1f) dt = 0.1f;

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
        lastPrint = now;
    }

    // update camera normal based on IMU (depends on BNO055)
    // do not read position from IMU

    // get joystick (x, y, z) -> remove y to project to get direction of
    // movement. change player position by delta (x, 0, z).

    // game rules
}
