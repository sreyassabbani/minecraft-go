#pragma once

#include <algebra.hpp>
#include <imu_sensor.hpp>
#include <world.hpp>

class Player {
    algebra::Quaternion orientation;
    Imu* imu;

public:
    algebra::Vector<3> position;
    algebra::Vector<3> velocity;

    // AABB dimensions
    const float width = 0.4f;
    const float height = 1.8f;

    explicit Player(Imu* imu = nullptr);
    Player(Imu* imu, algebra::Vector<3> startPos);

    // defer orientation calculations to the IMU
    algebra::Quaternion getOrientation();

    void applyGravity(float dt);
    void jump();
    void moveForward(float amount);
    void moveRight(float amount);
    void updatePhysics(World& world, float dt);
    void move();

private:
    void resolveCollisions(World& world, algebra::Vector<3>& vel, float dt);
    bool checkCollision(World& world, const algebra::Vector<3>& pos);
};
