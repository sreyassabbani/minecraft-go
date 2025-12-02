#pragma once

#include <algebra.hpp>
#include <stdint.h>

using Vec3 = algebra::Vector<3>;

// Triangle structure for rendering
// After projection and clipping, we rasterize triangles
struct Triangle {
    // Screen-space vertices (in pixels)
    int16_t x[3];
    int16_t y[3];

    // Depth for sorting (average Z in view space)
    float depth;

    // RGB565 color
    uint16_t color;

    // Visibility flag
    bool visible;
};

// Static face buffer - no dynamic allocation
// Maximum of 2000 triangles (fits in Arduino Due RAM)
class FaceList {
public:
    static const int MAX_TRIANGLES = 2000;

    FaceList();

    // Add a triangle to the list
    bool add(const Triangle& tri);

    // Clear all triangles
    void clear();

    // Get number of triangles
    int count() const { return triCount; }

    // Access triangle by index
    const Triangle& operator[](int index) const { return triangles[index]; }
    Triangle& operator[](int index) { return triangles[index]; }

    // Sort triangles by depth (far to near for painter's algorithm)
    void sortByDepth();

private:
    Triangle triangles[MAX_TRIANGLES];
    int triCount;
};