#include "algebra.h"

#include <cmath>

namespace algebra {
namespace {
constexpr float kHalfPi = 1.5707963267948966f;
}

Quaternion normalizeQuaternion(const Quaternion& q) {
    const float norm =
        std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (norm <= 1e-6f) { return Quaternion { 1.0f, 0.0f, 0.0f, 0.0f }; }
    const float inv = 1.0f / norm;
    return Quaternion { q.w * inv, q.x * inv, q.y * inv, q.z * inv };
}

Vector<3> rotateVector(const Quaternion& q, const Vector<3>& v) {
    const float qw = q.w, qx = q.x, qy = q.y, qz = q.z;

    const float rw = -qx * v[0] - qy * v[1] - qz * v[2];
    const float rx = qw * v[0] + qy * v[2] - qz * v[1];
    const float ry = qw * v[1] + qz * v[0] - qx * v[2];
    const float rz = qw * v[2] + qx * v[1] - qy * v[0];

    return Vector<3> { -rw * qx + rx * qw - ry * qz + rz * qy,
                       -rw * qy + ry * qw - rz * qx + rx * qz,
                       -rw * qz + rz * qw - rx * qy + ry * qx };
}

Vector<3> quaternionToEuler(const Quaternion& q) {
    const float qw = q.w, qx = q.x, qy = q.y, qz = q.z;

    const float sinr_cosp = 2.0f * (qw * qx + qy * qz);
    const float cosr_cosp = 1.0f - 2.0f * (qx * qx + qy * qy);
    const float roll = std::atan2f(sinr_cosp, cosr_cosp);

    const float sinp = 2.0f * (qw * qy - qz * qx);
    const float pitch = (std::fabs(sinp) >= 1.0f)
                            ? std::copysign(kHalfPi, sinp)
                            : std::asinf(sinp);

    const float siny_cosp = 2.0f * (qw * qz + qx * qy);
    const float cosy_cosp = 1.0f - 2.0f * (qy * qy + qz * qz);
    const float yaw = std::atan2f(siny_cosp, cosy_cosp);

    // yaw (Z), pitch (Y), roll (X)
    return Vector<3> { yaw, pitch, roll };
}
} // namespace algebra
