#pragma once

#include <Arduino.h>
#include <game_state.hpp>

struct InputPins {
    int joyX;
    int joyY;
    int sprintBtn;
    int jumpBtn;
    int placeBtn;
    int removeBtn;
};

class InputHandler {
public:
    InputHandler(InputPins pins, float baseSpeed = 1.0f,
                 float sprintMult = 2.0f, int deadzone = 50,
                 float reach = 4.0f);

    void begin() const;
    void update(Player& player, GameState& game, float dt);

private:
    InputPins pins_;
    float baseSpeed_;
    float sprintMult_;
    int deadzone_;
    float reach_;

    int lastJumpState_;
    int lastPlaceState_;
    int lastRemoveState_;

    float normAxis(int raw) const;
};
