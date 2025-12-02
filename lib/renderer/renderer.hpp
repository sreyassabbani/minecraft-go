#ifndef RENDERER_H
#define RENDERER_H

#include <camera.hpp>
#include <display.hpp>
#include <face_list.hpp>
#include <player.hpp>
#include <raster.hpp>

using Vec3 = algebra::Vector<3>;
using Vec4 = algebra::Vector<4>;
using Mat4 = algebra::Matrix<4, 4>;

class Renderer {
public:
    Renderer(display::Display& disp);

    // Main render function
    // should we actually pass in player or make it part of program
    // memory/static memory?
    // i also thought this should only take in cam_normal and that's it?
    // wouldn't that be better?
    void render(const World& world, const Player& player);

    // Test scene with single cube
    void renderTestScene();

private:
    display::Display& display;
    FaceList faceList;

    // Helper: multiply 4x4 matrix by 3D vector (assuming w=1)
    Vec4 multiplyMatrixVector(const Vec3& v, const Mat4& m);

    // Helper: multiply 4x4 matrix by 4D vector
    Vec4 multiplyMatrixVector(const Vec4& v, const Mat4& m);

    // Helper: compute face normal
    Vec3 computeNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2);

    // Helper: check if face should be culled (backface culling)
    // Returns true if face should be culled
    bool shouldCullFace(const Vec3& normal, const Vec3& faceCenter,
                        const Vec3& cameraPos);

    // Helper: project and add a quad to the face list
    // Splits quad into 2 triangles
    void projectAndAddQuad(const Vec3 verts[4], uint16_t color,
                           const Mat4& viewProj, const Camera& cam);

    // Helper: project a single vertex
    // Returns false if behind near plane
    bool projectVertex(const Vec3& worldPos, const Mat4& viewProj,
                       int16_t& outX, int16_t& outY, float& outZ);

    // Extract faces from world (Voxlap-style: only faces adjacent to air)
    void extractFaces(const World& world, const Camera& cam,
                      const Mat4& viewProj);
};

#endif
