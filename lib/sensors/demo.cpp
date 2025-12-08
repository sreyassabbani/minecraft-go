#include <demo.hpp>
#include <cmath>

DemoImu::DemoImu() = default;

void DemoImu::begin() { startMillis = millis(); }

void DemoImu::update() {
    const float t = (millis() - startMillis) / 1000.0f;

    const algebra::Vector<3> center { World::WIDTH * 0.5f, 1.5f,
                                      World::DEPTH * 0.5f };
    const float radius = 8.0f;
    const float height = 3.0f;
    const float angularSpeed = 0.4f; // radians per second
    const float angle = angularSpeed * t;

    position = algebra::Vector<3>({ center[0] + cosf(angle) * radius, height,
                                    center[2] + sinf(angle) * radius });
    velocity = algebra::Vector<3>(
        { -sinf(angle) * angularSpeed * radius, 0.0f,
          cosf(angle) * angularSpeed * radius });

    orientation = algebra::lookAt(position, center);
}

algebra::Vector<3> DemoImu::getLinearAcceleration() const {
    return algebra::Vector<3> { 0.0f, 0.0f, 0.0f };
}

algebra::Vector<3> DemoImu::getOrientationEuler() const {
    return algebra::quaternionToEuler(orientation);
}

algebra::Quaternion
DemoImu::getOrientation(const algebra::Quaternion& current) const {
    (void)current;
    return orientation;
}

algebra::Vector<3> DemoImu::getPosition() const { return position; }

algebra::Vector<3> DemoImu::getVelocity() const { return velocity; }
