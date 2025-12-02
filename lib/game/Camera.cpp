#include "Camera.h"
#include <math.h>

Camera::Camera() {
    position = Vec3({0.0f, 0.0f, 0.0f});
    lookDir = Vec3({0.0f, 0.0f, 1.0f});
    upDir = Vec3({0.0f, 1.0f, 0.0f});
    fov = 90.0f * 3.14159265f / 180.0f;
    aspectRatio = 1.5f;
    nearPlane = 0.1f;
    farPlane = 1000.0f;
}

Camera::Camera(const Vec3& pos, const Vec3& lookDirection, float fovDegrees, float aspect, float near, float far) {
    position = pos;
    lookDir = lookDirection;
    upDir = Vec3({0.0f, 1.0f, 0.0f});
    fov = fovDegrees * 3.14159265f / 180.0f;
    aspectRatio = aspect;
    nearPlane = near;
    farPlane = far;
    
    // Normalize look direction
    float len = sqrtf(lookDir[0]*lookDir[0] + lookDir[1]*lookDir[1] + lookDir[2]*lookDir[2]);
    if (len > 0.0001f) {
        lookDir[0] /= len;
        lookDir[1] /= len;
        lookDir[2] /= len;
    }
}

Mat4 Camera::getProjectionMatrix() const {
    // javidx9's projection matrix formulation
    // This is the standard perspective projection matrix
    Mat4 proj;
    
    float fovRad = fov;
    float fAspectRatio = aspectRatio;
    float fNear = nearPlane;
    float fFar = farPlane;
    float fFov = 1.0f / tanf(fovRad * 0.5f);
    
    proj(0, 0) = fAspectRatio * fFov;
    proj(1, 1) = fFov;
    proj(2, 2) = fFar / (fFar - fNear);
    proj(3, 2) = (-fFar * fNear) / (fFar - fNear);
    proj(2, 3) = 1.0f;
    proj(3, 3) = 0.0f;
    
    return proj;
}

Mat4 Camera::pointAt(const Vec3& pos, const Vec3& target, const Vec3& up) const {
    // Calculate new forward direction
    Vec3 newForward = target - pos;
    float len = sqrtf(newForward[0]*newForward[0] + newForward[1]*newForward[1] + newForward[2]*newForward[2]);
    if (len > 0.0001f) {
        newForward[0] /= len;
        newForward[1] /= len;
        newForward[2] /= len;
    }
    
    // Calculate new up direction
    // a = newForward dot up
    float a = newForward[0]*up[0] + newForward[1]*up[1] + newForward[2]*up[2];
    Vec3 newUp;
    newUp[0] = up[0] - a * newForward[0];
    newUp[1] = up[1] - a * newForward[1];
    newUp[2] = up[2] - a * newForward[2];
    
    len = sqrtf(newUp[0]*newUp[0] + newUp[1]*newUp[1] + newUp[2]*newUp[2]);
    if (len > 0.0001f) {
        newUp[0] /= len;
        newUp[1] /= len;
        newUp[2] /= len;
    }
    
    // New right direction = newUp x newForward
    Vec3 newRight;
    newRight[0] = newUp[1] * newForward[2] - newUp[2] * newForward[1];
    newRight[1] = newUp[2] * newForward[0] - newUp[0] * newForward[2];
    newRight[2] = newUp[0] * newForward[1] - newUp[1] * newForward[0];
    
    // Construct matrix
    Mat4 matrix;
    matrix(0, 0) = newRight[0];   matrix(0, 1) = newRight[1];   matrix(0, 2) = newRight[2];   matrix(0, 3) = 0.0f;
    matrix(1, 0) = newUp[0];      matrix(1, 1) = newUp[1];      matrix(1, 2) = newUp[2];      matrix(1, 3) = 0.0f;
    matrix(2, 0) = newForward[0]; matrix(2, 1) = newForward[1]; matrix(2, 2) = newForward[2]; matrix(2, 3) = 0.0f;
    matrix(3, 0) = pos[0];        matrix(3, 1) = pos[1];        matrix(3, 2) = pos[2];        matrix(3, 3) = 1.0f;
    
    return matrix;
}

Mat4 Camera::quickInverse(const Mat4& m) const {
    // Only for rotation/translation matrices
    Mat4 matrix;
    matrix(0, 0) = m(0, 0); matrix(0, 1) = m(1, 0); matrix(0, 2) = m(2, 0); matrix(0, 3) = 0.0f;
    matrix(1, 0) = m(0, 1); matrix(1, 1) = m(1, 1); matrix(1, 2) = m(2, 1); matrix(1, 3) = 0.0f;
    matrix(2, 0) = m(0, 2); matrix(2, 1) = m(1, 2); matrix(2, 2) = m(2, 2); matrix(2, 3) = 0.0f;
    matrix(3, 0) = -(m(3, 0) * matrix(0, 0) + m(3, 1) * matrix(1, 0) + m(3, 2) * matrix(2, 0));
    matrix(3, 1) = -(m(3, 0) * matrix(0, 1) + m(3, 1) * matrix(1, 1) + m(3, 2) * matrix(2, 1));
    matrix(3, 2) = -(m(3, 0) * matrix(0, 2) + m(3, 1) * matrix(1, 2) + m(3, 2) * matrix(2, 2));
    matrix(3, 3) = 1.0f;
    return matrix;
}

Mat4 Camera::getViewMatrix() const {
    // Point camera in the direction it's looking
    Vec3 target = position + lookDir;
    Mat4 matCamera = pointAt(position, target, upDir);
    
    // View matrix is inverse of camera matrix
    return quickInverse(matCamera);
}
