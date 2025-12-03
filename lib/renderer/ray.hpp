#pragma once

#include <algebra.hpp>
#include <world.hpp>

struct RaycastHit {
    bool hit;
    int x, y, z;
    int face; // 0: -x, 1: +x, 2: -y, 3: +y, 4: -z, 5: +z
};

RaycastHit raycast(const algebra::Vector<3>& start,
                   const algebra::Vector<3>& dir, float maxDist,
                   const World& world);
