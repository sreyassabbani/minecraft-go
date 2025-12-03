#include <math.h>
#include <ray.hpp>

// Helper for sign function
template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

RaycastHit raycast(const algebra::Vector<3>& start,
                   const algebra::Vector<3>& dir, float maxDist,
                   const World& world) {
    RaycastHit result = { false, 0, 0, 0, 0 };

    float x = start[0];
    float y = start[1];
    float z = start[2];

    int mapX = (int)floor(x);
    int mapY = (int)floor(y);
    int mapZ = (int)floor(z);

    float deltaDistX = fabs(1.0f / dir[0]);
    float deltaDistY = fabs(1.0f / dir[1]);
    float deltaDistZ = fabs(1.0f / dir[2]);

    int stepX = sgn(dir[0]);
    int stepY = sgn(dir[1]);
    int stepZ = sgn(dir[2]);

    float sideDistX =
        (stepX > 0) ? (mapX + 1.0f - x) * deltaDistX : (x - mapX) * deltaDistX;
    float sideDistY =
        (stepY > 0) ? (mapY + 1.0f - y) * deltaDistY : (y - mapY) * deltaDistY;
    float sideDistZ =
        (stepZ > 0) ? (mapZ + 1.0f - z) * deltaDistZ : (z - mapZ) * deltaDistZ;

    int side = 0; // 0 for x-hit, 1 for y-hit, 2 for z-hit

    // We need to track distance traveled
    float dist = 0.0f;

    while (dist < maxDist) {
        if (sideDistX < sideDistY) {
            if (sideDistX < sideDistZ) {
                dist = sideDistX;
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                dist = sideDistZ;
                sideDistZ += deltaDistZ;
                mapZ += stepZ;
                side = 2;
            }
        } else {
            if (sideDistY < sideDistZ) {
                dist = sideDistY;
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            } else {
                dist = sideDistZ;
                sideDistZ += deltaDistZ;
                mapZ += stepZ;
                side = 2;
            }
        }

        if (world.inBounds(mapX, mapY, mapZ)) {
            if (world.isSolid(world.getBlock(mapX, mapY, mapZ))) {
                result.hit = true;
                result.x = mapX;
                result.y = mapY;
                result.z = mapZ;

                // Determine face
                if (side == 0)
                    result.face =
                        (stepX > 0)
                            ? 0
                            : 1; // hit from left (-x) or right (+x) side? Wait.
                // If stepX > 0, we are moving in +x direction. We hit the left
                // face of the block (which faces -x). So if stepX > 0, face is
                // -x (0). If stepX < 0, face is +x (1).
                if (side == 0) result.face = (stepX > 0) ? 0 : 1;
                else if (side == 1)
                    result.face = (stepY > 0) ? 2 : 3;  // 2: -y, 3: +y
                else result.face = (stepZ > 0) ? 4 : 5; // 4: -z, 5: +z

                return result;
            }
        } else {
            // Out of bounds, stop? Or continue until maxDist?
            // For now, if we go out of bounds, we might re-enter?
            // But for a bounded world, once out, usually out.
            // Let's check if we are moving away from the world box.
            // Simple check: if mapX < -1 or mapX > WIDTH etc.
            // For now, just continue.
        }
    }

    return result;
}
