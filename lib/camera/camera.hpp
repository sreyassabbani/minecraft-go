#ifndef CAMERA_H
#define CAMERA_H

#include <algebra.hpp>

// Typedefs for clarity
using Vec3 = algebra::Vector<3>;
using Vec4 = algebra::Vector<4>;
using Mat4 = algebra::Matrix<4, 4>;

// Camera class based on javidx9's "3D Graphics Engine" projection approach
class Camera {
public:
    Vec3 position;
    Vec3 lookDir; // Direction camera is looking (normalized)
    Vec3 upDir;   // Up direction (normalized)

    float fov; // Field of view (vertical, in radians)
    float aspectRatio;
    float nearPlane;
    float farPlane;

    Camera();
    Camera(const Vec3& pos, const Vec3& lookDirection, float fovDegrees,
           float aspect, float near, float far);

    // Get view matrix (world to camera space transform)
    Mat4 getViewMatrix() const;

    // Get projection matrix (camera space to clip space)
    // Based on javidx9's projection matrix formulation
    Mat4 getProjectionMatrix() const;

private:
    // Helper to construct view matrix from camera vectors
    Mat4 pointAt(const Vec3& pos, const Vec3& target, const Vec3& up) const;

    // Quick inverse for rotation+translation only matrices
    Mat4 quickInverse(const Mat4& m) const;
};

#endif
