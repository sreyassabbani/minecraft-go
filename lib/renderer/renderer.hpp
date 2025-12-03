#pragma once

#include <blocks.hpp>
#include <camera.hpp>
#include <display.hpp>
#include <face_list.hpp>
#include <raster.hpp>
#include <world.hpp>

using Vec3 = algebra::Vector<3>;
using Vec4 = algebra::Vector<4>;
using Mat4 = algebra::Matrix<4, 4>;

class Renderer {
public:
    Renderer(display::Display& disp);

    // Main render function
    // Camera pose provided explicitly to keep renderer decoupled from Player
    void render(const World& world, const Vec3& camPos,
                const algebra::Quaternion& camOrientation);

private:
    display::Display& display;
    FaceList faceList;

    // Helper: check if face should be culled (backface culling)
    // Returns true if face should be culled
    bool shouldCullFace(const Vec3& normal, const Vec3& faceCenter,
                        const Vec3& cameraPos);

    // Helper: project and add a quad to the face list
    // Splits quad into 2 triangles
    void projectAndAddQuad(const Vec3 verts[4], uint16_t color,
                           const Mat4& view, const Mat4& viewProj,
                           const Camera& cam);

    // Helper: project a single vertex
    // Returns false if behind near plane
    bool projectVertex(const Vec3& worldPos, const Mat4& view,
                       const Mat4& viewProj, int16_t& outX, int16_t& outY,
                       float& outZ);

    // Extract faces from world (Voxlap-style: only faces adjacent to air)
    void extractFaces(const World& world, const Camera& cam, const Mat4& view,
                      const Mat4& viewProj);
};
