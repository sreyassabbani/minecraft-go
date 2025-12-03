#include <general.hpp>
#include <math.h>
#include <renderer.hpp>

Renderer::Renderer(display::Display& disp) : display(disp) {}

void Renderer::render(const World& world, const Vec3& camPos,
                      const algebra::Quaternion& camOrientation) {
    // Derive camera basis from player orientation so view follows head pose
    Vec3 forward =
        algebra::rotateVector(camOrientation, Vec3({ 0.0f, 0.0f, 1.0f }));
    Vec3 up =
        algebra::rotateVector(camOrientation, Vec3({ 0.0f, 1.0f, 0.0f }));

    forward = algebra::normalizeOr(forward, Vec3({ 0.0f, 0.0f, 1.0f }));
    up = algebra::normalizeOr(up, Vec3({ 0.0f, 1.0f, 0.0f }));

    float aspect = static_cast<float>(display.width()) /
                   static_cast<float>(display.height());

    Camera cam(camPos, forward, 70.0f, aspect, 0.1f, 100.0f);
    cam.upDir = up;

    // Get view and projection matrices
    Mat4 view = cam.getViewMatrix();
    Mat4 proj = cam.getProjectionMatrix();
    Mat4 viewProj = view * proj; // row-vector convention: v * view * proj

    // Clear screen
    display.clear(display::Color::SkyBlue());

    // Clear face list
    faceList.clear();

    // Extract visible faces from world
    extractFaces(world, cam, view, viewProj);

    // Debug: report face count
    println("Faces extracted: ", faceList.count());
    if (faceList.count() == 0) {
        println("No faces extracted - check camera/view/projection");
    }

    // Sort faces by depth
    faceList.sortByDepth();

    // Rasterize all faces
    const int16_t screenW = display.width();
    const int16_t screenH = display.height();
    int rendered = 0;
    for (int i = 0; i < faceList.count(); i++) {
        const Triangle& tri = faceList[i];
        if (tri.visible) {
            int16_t minX = tri.x[0], maxX = tri.x[0];
            int16_t minY = tri.y[0], maxY = tri.y[0];
            for (int v = 1; v < 3; ++v) {
                if (tri.x[v] < minX) minX = tri.x[v];
                if (tri.x[v] > maxX) maxX = tri.x[v];
                if (tri.y[v] < minY) minY = tri.y[v];
                if (tri.y[v] > maxY) maxY = tri.y[v];
            }
            if (maxX < 0 || minX >= screenW || maxY < 0 || minY >= screenH) {
                continue; // triangle wholly off-screen
            }

            Raster::fillTriangle(display, tri.x[0], tri.y[0], tri.x[1],
                                 tri.y[1], tri.x[2], tri.y[2], tri.color);

            rendered++;
        }
    }
}

void Renderer::renderTestScene() {
    // Camera positioned to look directly at cube
    // Cube is at (0, 0, 4.5), camera at (0, 0, -3) looking forward
    Camera cam(Vec3({ 0.0f, 0.0f, -3.0f }), Vec3({ 0.0f, 0.0f, 1.0f }), 70.0f,
               1.5f, 0.1f, 100.0f);

    // Get matrices
    Mat4 view = cam.getViewMatrix();
    Mat4 proj = cam.getProjectionMatrix();
    Mat4 viewProj = view * proj; // row-vector convention: v * view * proj

    Serial.println("=== renderTestScene ===");

    // Clear screen
    display.clear(display::Color::Black());

    // Clear face list
    faceList.clear();

    // Define a cube further from camera (to avoid near-plane clipping)
    // Cube will be at Z=4 to Z=5 (7-8 units from camera at Z=-3)
    float x0 = -0.5f, x1 = 0.5f;
    float y0 = -0.5f, y1 = 0.5f;
    float z0 = 4.0f, z1 = 5.0f;

    // Front face (+Z)
    Vec3 frontVerts[4] = {
        { x0, y0, z1 },
        { x1, y0, z1 },
        { x1, y1, z1 },
        { x0, y1, z1 }
    };
    projectAndAddQuad(frontVerts, 0xF800, view, viewProj, cam); // Red

    // Back face (-Z)
    Vec3 backVerts[4] = {
        { x1, y0, z0 },
        { x0, y0, z0 },
        { x0, y1, z0 },
        { x1, y1, z0 }
    };
    projectAndAddQuad(backVerts, 0x07E0, view, viewProj, cam); // Green

    // Right face (+X)
    Vec3 rightVerts[4] = {
        { x1, y0, z1 },
        { x1, y0, z0 },
        { x1, y1, z0 },
        { x1, y1, z1 }
    };
    projectAndAddQuad(rightVerts, 0x001F, view, viewProj, cam); // Blue

    // Left face (-X)
    Vec3 leftVerts[4] = {
        { x0, y0, z0 },
        { x0, y0, z1 },
        { x0, y1, z1 },
        { x0, y1, z0 }
    };
    projectAndAddQuad(leftVerts, 0xFFE0, view, viewProj, cam); // Yellow

    // Top face (+Y)
    Vec3 topVerts[4] = {
        { x0, y1, z1 },
        { x1, y1, z1 },
        { x1, y1, z0 },
        { x0, y1, z0 }
    };
    projectAndAddQuad(topVerts, 0xF81F, view, viewProj, cam); // Magenta

    // Bottom face (-Y)
    Vec3 bottomVerts[4] = {
        { x0, y0, z0 },
        { x1, y0, z0 },
        { x1, y0, z1 },
        { x0, y0, z1 }
    };
    projectAndAddQuad(bottomVerts, 0x07FF, view, viewProj, cam); // Cyan

    Serial.print("Face count: ");
    Serial.println(faceList.count());

    // Sort faces by depth
    faceList.sortByDepth();

    // Rasterize all faces
    for (int i = 0; i < faceList.count(); i++) {
        const Triangle& tri = faceList[i];
        if (tri.visible) {
            Raster::fillTriangle(display, tri.x[0], tri.y[0], tri.x[1],
                                 tri.y[1], tri.x[2], tri.y[2], tri.color);
        }
    }

    Serial.println("=== renderTestScene complete ===");
}

bool Renderer::shouldCullFace(const Vec3& normal, const Vec3& faceCenter,
                              const Vec3& cameraPos) {
    // Vector from face to camera
    Vec3 viewDir;
    viewDir[0] = cameraPos[0] - faceCenter[0];
    viewDir[1] = cameraPos[1] - faceCenter[1];
    viewDir[2] = cameraPos[2] - faceCenter[2];

    // Dot product
    float dot = normal[0] * viewDir[0] + normal[1] * viewDir[1] +
                normal[2] * viewDir[2];

    // If dot < 0, face is pointing away from camera
    return dot <= 0.0f;
}

bool Renderer::projectVertex(const Vec3& worldPos, const Mat4& view,
                             const Mat4& viewProj, int16_t& outX,
                             int16_t& outY, float& outZ) {
    // Transform to view space for depth/near-plane checks
    Vec4 viewPos = worldPos * view;

    // Transform to clip space
    Vec4 clipPos = worldPos * viewProj;

    // Debug one block projection to see if it's culled
    static bool loggedSample = false;
    if (!loggedSample && fabs(worldPos[0] - 5.0f) < 0.1f &&
        fabs(worldPos[1] - 1.0f) < 0.1f && fabs(worldPos[2] - 5.0f) < 0.1f) {
        println("Sample block clip: (", clipPos[0], ",", clipPos[1], ",",
                clipPos[2], ", W=", clipPos[3], ")");
        loggedSample = true;
    }

    // Clip against near plane using clip-space W
    if (clipPos[3] <= 0.0f) { return false; }

    // Perspective divide
    float invW = 1.0f / clipPos[3];
    float ndcX = clipPos[0] * invW;
    float ndcY = clipPos[1] * invW;

    // Debug center block projection (approximate check)
    if (abs(worldPos[0] - 5.0f) < 0.1f && abs(worldPos[2]) < 0.1f) {
        Serial.print("Proj (5,y,0): NDC=(");
        Serial.print(ndcX);
        Serial.print(",");
        Serial.print(ndcY);
        Serial.print(") W=");
        Serial.println(clipPos[3]);
    }

    // Convert to screen space
    float screenX = (ndcX + 1.0f) * 0.5f * (float)display.width();
    float screenY = (1.0f - ndcY) * 0.5f * (float)display.height();

    // Clamp to prevent overflow
    if (screenX < -2000.0f) screenX = -2000.0f;
    if (screenX > 2480.0f) screenX = 2480.0f;
    if (screenY < -2000.0f) screenY = -2000.0f;
    if (screenY > 2320.0f) screenY = 2320.0f;

    outX = (int16_t)screenX;
    outY = (int16_t)screenY;
    outZ = viewPos[2]; // Use view-space Z for depth sorting

    return true;
}

void Renderer::projectAndAddQuad(const Vec3 verts[4], uint16_t color,
                                 const Mat4& view, const Mat4& viewProj,
                                 const Camera& cam) {
    // Compute face center
    Vec3 center;
    center[0] = (verts[0][0] + verts[1][0] + verts[2][0] + verts[3][0]) * 0.25f;
    center[1] = (verts[0][1] + verts[1][1] + verts[2][1] + verts[3][1]) * 0.25f;
    center[2] = (verts[0][2] + verts[1][2] + verts[2][2] + verts[3][2]) * 0.25f;

    // Compute normal
    Vec3 normal = algebra::faceNormal(verts[0], verts[1], verts[2]);

    // Backface culling
    if (shouldCullFace(normal, center, cam.position)) { return; }

    // Project vertices
    int16_t screenX[4], screenY[4];
    float depth[4];
    int validVerts = 0;

    for (int i = 0; i < 4; i++) {
        if (projectVertex(verts[i], view, viewProj, screenX[i], screenY[i],
                          depth[i])) {
            validVerts++;
        }
    }

    // If any vertices are behind camera, skip (simplified clipping)
    if (validVerts < 4) { return; }

    // Split quad into two triangles: 0-1-2 and 0-2-3
    float avgDepth1 = (depth[0] + depth[1] + depth[2]) / 3.0f;
    float avgDepth2 = (depth[0] + depth[2] + depth[3]) / 3.0f;

    // Triangle 1: 0-1-2
    Triangle tri1;
    tri1.x[0] = screenX[0];
    tri1.y[0] = screenY[0];
    tri1.x[1] = screenX[1];
    tri1.y[1] = screenY[1];
    tri1.x[2] = screenX[2];
    tri1.y[2] = screenY[2];
    tri1.depth = avgDepth1;
    tri1.color = color;
    tri1.visible = true;
    faceList.add(tri1);

    // Triangle 2: 0-2-3
    Triangle tri2;
    tri2.x[0] = screenX[0];
    tri2.y[0] = screenY[0];
    tri2.x[1] = screenX[2];
    tri2.y[1] = screenY[2];
    tri2.x[2] = screenX[3];
    tri2.y[2] = screenY[3];
    tri2.depth = avgDepth2;
    tri2.color = color;
    tri2.visible = true;
    faceList.add(tri2);
}

void Renderer::extractFaces(const World& world, const Camera& cam,
                            const Mat4& view, const Mat4& viewProj) {
    // Voxlap-style face extraction: only emit faces adjacent to air
    for (int x = 0; x < World::WIDTH; x++) {
        for (int y = 0; y < World::HEIGHT; y++) {
            for (int z = 0; z < World::DEPTH; z++) {
                uint8_t block = world.getBlock(x, y, z);
                if (block == AIR) continue;

                float worldX = static_cast<float>(x);
                float worldY = static_cast<float>(y);
                float worldZ = static_cast<float>(z);

                // Define vertices relative to block origin
                float x0 = worldX, x1 = worldX + 1.0f;
                float y0 = worldY, y1 = worldY + 1.0f;
                float z0 = worldZ, z1 = worldZ + 1.0f;

                // Check 6 neighbors
                struct {
                    int dx, dy, dz;
                } dirs[] = {
                    { 0,  0,  1  },
                    { 0,  0,  -1 }, // Front, Back
                    { 1,  0,  0  },
                    { -1, 0,  0  }, // Right, Left
                    { 0,  1,  0  },
                    { 0,  -1, 0  }  // Top, Bottom
                };

                for (const auto& d : dirs) {
                    int nx = x + d.dx;
                    int ny = y + d.dy;
                    int nz = z + d.dz;

                    // Only draw face if neighbor is air or out of bounds
                    bool drawFace = !world.inBounds(nx, ny, nz) ||
                                    world.getBlock(nx, ny, nz) == AIR;

                    if (!drawFace) continue;

                    // Define quad vertices based on direction
                    Vec3 verts[4];
                    uint16_t color;

                    if (d.dz == 1) { // Front (+Z)
                        verts[0] = { x0, y0, z1 };
                        verts[1] = { x1, y0, z1 };
                        verts[2] = { x1, y1, z1 };
                        verts[3] = { x0, y1, z1 };
                    } else if (d.dz == -1) { // Back (-Z)
                        verts[0] = { x1, y0, z0 };
                        verts[1] = { x0, y0, z0 };
                        verts[2] = { x0, y1, z0 };
                        verts[3] = { x1, y1, z0 };
                    } else if (d.dx == 1) { // Right (+X)
                        verts[0] = { x1, y0, z1 };
                        verts[1] = { x1, y0, z0 };
                        verts[2] = { x1, y1, z0 };
                        verts[3] = { x1, y1, z1 };
                    } else if (d.dx == -1) { // Left (-X)
                        verts[0] = { x0, y0, z0 };
                        verts[1] = { x0, y0, z1 };
                        verts[2] = { x0, y1, z1 };
                        verts[3] = { x0, y1, z0 };
                    } else if (d.dy == 1) { // Top (+Y)
                        verts[0] = { x0, y1, z1 };
                        verts[1] = { x1, y1, z1 };
                        verts[2] = { x1, y1, z0 };
                        verts[3] = { x0, y1, z0 };
                    } else { // Bottom (-Y)
                        verts[0] = { x0, y0, z0 };
                        verts[1] = { x1, y0, z0 };
                        verts[2] = { x1, y0, z1 };
                        verts[3] = { x0, y0, z1 };
                    }

                    // Assign color based on block type
                    if (block == DIRT) {
                        color = 0x79E0; // Brown
                    } else if (block == GRASS) {
                        if (d.dy == 1) color = 0x07E0;       // Green top
                        else if (d.dy == -1) color = 0x79E0; // Brown bottom
                        else color = 0x7BE0;                 // Green-brown side
                    } else if (block == STONE) {
                        color = 0x8410; // Gray
                    } else {
                        color = 0xFFFF; // White
                    }

                    // Project and add quad
                    projectAndAddQuad(verts, color, view, viewProj, cam);
                }
            }
        }
    }
}
