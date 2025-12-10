#include <math.h>
#include <algorithm>
#include <player.hpp>
#include <general.hpp>
#include <Arduino.h>

#define JOYSTICK_X A6
#define JOYSTICK_Y A7
#define JOYSTICK_BUTTON_PIN 53

Player::Player(Imu* imu)
    : Player(imu, algebra::Vector<3>({ 4.5f, 1.05f, 4.5f })) {}

Player::Player(Imu* imu, algebra::Vector<3> startPos)
    : orientation { 1.0f, 0.0f, 0.0f, 0.0f }, imu(imu), position(startPos),
      velocity(algebra::Vector<3>({ 0.0f, 0.0f, 0.0f })) {
    // Face the center of the world by default so the camera isn’t pointed at
    // the sky/ground before IMU data arrives.
    const algebra::Vector<3> target {
        World::WIDTH * 0.5f, 1.5f, World::DEPTH * 0.5f
    };
    orientation = algebra::lookAt(position, target);
}

void Player::applyGravity(float dt) { velocity[1] -= 9.81f * dt; }

algebra::Quaternion Player::getOrientation() {
    if (imu && imu->hasOrientation()) {
        orientation = imu->getOrientation(orientation);
    }
    return orientation;
}

void Player::jump() {
    // Simple jump, assuming on ground check is done elsewhere or allowed to air
    // jump for now. Explicitly bump Y to satisfy the requested jump height.
    position[1] += 5.0f;
    velocity[1] = 0.0f;
}

void Player::moveForward(float amount) {
    // Forward is +Z in local space, but we need to rotate by orientation.
    // Actually, usually Forward is -Z in OpenGL, but requirements say:
    // "forward/backward is orientation’s +Z"

    const algebra::Quaternion orientation = getOrientation();

    algebra::Vector<3> forward = algebra::Vector<3>({ 0.0f, 0.0f, 1.0f });
    forward = algebra::rotateVector(orientation, forward);

    // Flatten forward vector to XZ plane for walking
    forward[1] = 0.0f;

    // Normalize if length > 0
    float len = sqrt(forward[0] * forward[0] + forward[2] * forward[2]);
    if (len > 0.001f) {
        forward[0] /= len;
        forward[2] /= len;
    }

    position = position + forward * amount;
}

void Player::moveRight(float amount) {
    // Right is +X
    const algebra::Quaternion orientation = getOrientation();

    algebra::Vector<3> right = algebra::Vector<3>({ 1.0f, 0.0f, 0.0f });
    right = algebra::rotateVector(orientation, right);

    right[1] = 0.0f;
    float len = sqrt(right[0] * right[0] + right[2] * right[2]);
    if (len > 0.001f) {
        right[0] /= len;
        right[2] /= len;
    }

    position = position + right * amount;
}

void Player::move() {
    int jX = analogRead(JOYSTICK_X);
    int jY = analogRead(JOYSTICK_Y);

    println("Joystick Readings - X:", String(jX), " Y:", String(jY));
    
    float jX1 =( (float) jX - 512.0f) / 512.0f;
    float jY1 =( (float) jY - 512.0f) / 512.0f;

    moveForward(jY1);
    moveRight(jX1);
}

bool Player::checkCollision(World& world, const algebra::Vector<3>& pos) {
    // Check AABB against blocks
    // Player AABB: [pos.x - w/2, pos.x + w/2] x [pos.y, pos.y + h] x [pos.z -
    // w/2, pos.z + w/2]

    float minX = pos[0] - width / 2.0f;
    float maxX = pos[0] + width / 2.0f;
    float minY = pos[1];
    float maxY = pos[1] + height;
    float minZ = pos[2] - width / 2.0f;
    float maxZ = pos[2] + width / 2.0f;

    int startX = (int)floor(minX);
    int endX = (int)floor(maxX);
    int startY = (int)floor(minY);
    int endY = (int)floor(maxY);
    int startZ = (int)floor(minZ);
    int endZ = (int)floor(maxZ);

    for (int x = startX; x <= endX; ++x) {
        for (int y = startY; y <= endY; ++y) {
            for (int z = startZ; z <= endZ; ++z) {
                if (world.inBounds(x, y, z) &&
                    world.isSolid(world.getBlock(x, y, z))) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Player::updatePhysics(World& world, float dt) {
    applyGravity(dt);

    // Apply velocity to position with collision detection
    // Separate axes for better collision response

    // X axis
    algebra::Vector<3> nextPos = position;
    nextPos[0] += velocity[0] * dt;
    if (checkCollision(world, nextPos)) {
        velocity[0] = 0; // Stop
    } else {
        position[0] = nextPos[0];
    }

    // Z axis
    nextPos = position;
    nextPos[2] += velocity[2] * dt;
    if (checkCollision(world, nextPos)) {
        velocity[2] = 0;
    } else {
        position[2] = nextPos[2];
    }

    // Y axis
    nextPos = position;
    nextPos[1] += velocity[1] * dt;
    if (checkCollision(world, nextPos)) {
        if (velocity[1] < 0.0f) {
            // Snap to just above the block we hit to avoid sinking
            position[1] = (std::max)(0.0f, floorf(position[1]) + 0.001f);
        }
        velocity[1] = 0;
    } else {
        // Prevent tunneling below the world floor
        if (nextPos[1] < 0.0f) {
            nextPos[1] = 0.0f;
            velocity[1] = 0.0f;
        }
        position[1] = nextPos[1];
    }

    // Keep player above a minimum Y of 2
    if (position[1] < 2.0f) {
        position[1] = 2.0f;
        if (velocity[1] < 0.0f) velocity[1] = 0.0f;
    }

    // Friction/Damping on XZ
    velocity[0] *= 0.9f;
    velocity[2] *= 0.9f;
}
