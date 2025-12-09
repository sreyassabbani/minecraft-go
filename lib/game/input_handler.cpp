#include <general.hpp>
#include <input_handler.hpp>
#include <math.h>

InputHandler::InputHandler(InputPins pins, float baseSpeed, float sprintMult,
                           int deadzone, float reach)
    : pins_(pins), baseSpeed_(baseSpeed), sprintMult_(sprintMult),
      deadzone_(deadzone), reach_(reach), lastJumpState_(HIGH),
      lastPlaceState_(HIGH), lastRemoveState_(HIGH) {}

void InputHandler::begin() const {
    pinMode(pins_.sprintBtn, INPUT_PULLUP);
    pinMode(pins_.jumpBtn, INPUT_PULLUP);
    pinMode(pins_.placeBtn, INPUT_PULLUP);
    pinMode(pins_.removeBtn, INPUT_PULLUP);
}

float InputHandler::normAxis(int raw) const {
    const int centered = raw - 512;
    if (abs(centered) < deadzone_) return 0.0f;
    return static_cast<float>(centered) / 512.0f;
}

void InputHandler::update(Player& player, GameState& game, float dt) {
    const int rawX = analogRead(pins_.joyX);
    const int rawY = analogRead(pins_.joyY);
    const int sprintBtn = digitalRead(pins_.sprintBtn);
    const int jumpBtn = digitalRead(pins_.jumpBtn);
    const int placeBtn = digitalRead(pins_.placeBtn);
    const int removeBtn = digitalRead(pins_.removeBtn);

    // Swap axes to match wiring: X pin drives forward/back, Y pin drives strafe.
    const float joyStrafe = -normAxis(rawY);   // left/right (invert so left is negative)
    const float joyForward = -normAxis(rawX);  // forward/back (invert so up is forward)

    float speed = baseSpeed_;
    if (sprintBtn == LOW) { speed *= sprintMult_; }

    if (fabsf(joyForward) > 0.0f) { player.moveForward(joyForward * speed * dt); }
    if (fabsf(joyStrafe) > 0.0f) { player.moveRight(joyStrafe * speed * dt); }

    if (lastJumpState_ == HIGH && jumpBtn == LOW) { player.jump(); }
    lastJumpState_ = jumpBtn;

    if (lastPlaceState_ == HIGH && placeBtn == LOW) {
        game.placeStoneForward(player, reach_);
    }
    lastPlaceState_ = placeBtn;

    if (lastRemoveState_ == HIGH && removeBtn == LOW) {
        game.removeBlockForward(player, reach_);
    }
    lastRemoveState_ = removeBtn;
}
