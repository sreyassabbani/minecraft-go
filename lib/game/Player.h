#ifndef PLAYER_H
#define PLAYER_H

#include "../../lib/algebra/algebra.hpp"
#include "World.h"

class Player {
public:
    algebra::Vector<3> position;
    algebra::Vector<3> velocity;
    algebra::Quaternion orientation;

    // AABB dimensions
    const float width = 0.4f;
    const float height = 1.8f;

    Player();

    void applyGravity(float dt);
    void jump();
    void moveForward(float amount);
    void moveRight(float amount);
    void updatePhysics(World& world, float dt);

private:
    void resolveCollisions(World& world, algebra::Vector<3>& vel, float dt);
    bool checkCollision(World& world, const algebra::Vector<3>& pos);
};

#endif
