#include <math.h>
#include <player.hpp>

Player::Player(Imu* imu)
    : Player(imu, algebra::Vector<3>({ 5.0f, 10.0f, 5.0f })) {}

Player::Player(Imu* imu, algebra::Vector<3> startPos)
    : orientation { 1.0f, 0.0f, 0.0f, 0.0f }, imu(imu), position(startPos),
      velocity(algebra::Vector<3>({ 0.0f, 0.0f, 0.0f })) {}

void Player::applyGravity(float dt) { velocity[1] -= 9.81f * dt; }

algebra::Quaternion Player::getOrientation() {
    if (imu) {
        orientation = imu->getOrientation(orientation);
    }
    return orientation;
}

void Player::jump() {
    // Simple jump, assuming on ground check is done elsewhere or allowed to air
    // jump for now For better physics, check if on ground. For this step, let's
    // just add upward velocity.
    velocity[1] = 5.0f;
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

    position = position + forward * (amount * 0.1f); // Scale movement
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

    position = position + right * (amount * 0.1f);
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
        velocity[1] = 0;
        // If falling and hit ground, stop
    } else {
        position[1] = nextPos[1];
    }

    // Friction/Damping on XZ
    velocity[0] *= 0.9f;
    velocity[2] *= 0.9f;
}
