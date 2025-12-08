#include <algebra.hpp>
#include <cmath>

namespace algebra {
namespace {
constexpr float kHalfPi = 1.5707963267948966f;
}

Quaternion normalizeQuaternion(const Quaternion& q) {
    const float norm = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if (norm <= 1e-6f) { return Quaternion { 1.0f, 0.0f, 0.0f, 0.0f }; }
    const float inv = 1.0f / norm;
    return Quaternion { q.w * inv, q.x * inv, q.y * inv, q.z * inv };
}

Vector<3> cross(const Vector<3>& a, const Vector<3>& b) {
    return Vector<3> { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                       a[0] * b[1] - a[1] * b[0] };
}

float length(const Vector<3>& v) {
    return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

Vector<3> normalize(const Vector<3>& v) {
    const float len = length(v);
    if (len < 1e-6f) { return Vector<3> { 0.0f, 0.0f, 0.0f }; }
    const float inv = 1.0f / len;
    return Vector<3> { v[0] * inv, v[1] * inv, v[2] * inv };
}

Vector<3> normalizeOr(const Vector<3>& v, const Vector<3>& fallback) {
    const float len = length(v);
    if (len < 1e-6f) { return fallback; }
    const float inv = 1.0f / len;
    return Vector<3> { v[0] * inv, v[1] * inv, v[2] * inv };
}

Vector<3> faceNormal(const Vector<3>& v0, const Vector<3>& v1,
                     const Vector<3>& v2) {
    return normalize(cross(v1 - v0, v2 - v0));
}

Quaternion makeYawPitch(float yaw, float pitch) {
    const float hy = yaw * 0.5f;
    const float hp = pitch * 0.5f;
    const float cy = std::cos(hy);
    const float sy = std::sin(hy);
    const float cp = std::cos(hp);
    const float sp = std::sin(hp);
    return Quaternion { cy * cp, cy * sp, sy * cp, -sy * sp };
}

Quaternion lookAt(const Vector<3>& pos, const Vector<3>& target) {
    Vector<3> dir { target[0] - pos[0], target[1] - pos[1],
                    target[2] - pos[2] };
    const float len =
        std::sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
    if (len < 1e-4f) { return Quaternion { 1.0f, 0.0f, 0.0f, 0.0f }; }
    dir[0] /= len;
    dir[1] /= len;
    dir[2] /= len;

    const float yaw =
        std::atan2(dir[0], dir[2]); // rotate around Y to face target
    const float pitch =
        std::atan2(-dir[1], std::sqrt(dir[0] * dir[0] + dir[2] * dir[2]));
    return makeYawPitch(yaw, pitch);
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
    const float roll = std::atan2(sinr_cosp, cosr_cosp);

    const float sinp = 2.0f * (qw * qy - qz * qx);
    const float pitch = (std::fabs(sinp) >= 1.0f) ? std::copysign(kHalfPi, sinp)
                                                  : std::asin(sinp);

    const float siny_cosp = 2.0f * (qw * qz + qx * qy);
    const float cosy_cosp = 1.0f - 2.0f * (qy * qy + qz * qz);
    const float yaw = std::atan2(siny_cosp, cosy_cosp);

    // yaw (Z), pitch (Y), roll (X)
    return Vector<3> { yaw, pitch, roll };
}
} // namespace algebra
