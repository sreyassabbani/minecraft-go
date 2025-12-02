#include "Renderer.h"
#include <math.h>

Renderer::Renderer(display::Display& disp) : display(disp) {
}

void Renderer::render(const World& world, const Player& player) {
    // Setup camera - use simple forward direction like test cube
    Vec3 lookDir = Vec3({0.0f, 0.0f, 1.0f}); // Looking forward
    Camera cam(player.position, lookDir, 70.0f, 1.5f, 0.1f, 100.0f);
    
    // Get view and projection matrices
    Mat4 view = cam.getViewMatrix();
    Mat4 proj = cam.getProjectionMatrix();
    Mat4 viewProj = proj * view;
    
    // Clear screen
    display.clear(display::Color::Black());
    
    // Clear face list
    faceList.clear();
    
    Serial.println("--- Render Start ---");
    Serial.print("Camera at: "); 
    Serial.print(cam.position[0]); Serial.print(", ");
    Serial.print(cam.position[1]); Serial.print(", ");
    Serial.println(cam.position[2]);
    
    // Extract visible faces from world
    extractFaces(world, cam, viewProj);
    
    Serial.print("Faces extracted: ");
    Serial.println(faceList.count());
    
    // Sort faces by depth
    faceList.sortByDepth();
    
    // Rasterize all faces
    int rendered = 0;
    int onScreen = 0;
    for (int i = 0; i < faceList.count(); i++) {
        const Triangle& tri = faceList[i];
        if (tri.visible) {
            // Check if on screen
            bool isOnScreen = (tri.x[0] >= 0 && tri.x[0] < display.width() && tri.y[0] >= 0 && tri.y[0] < display.height()) ||
                              (tri.x[1] >= 0 && tri.x[1] < display.width() && tri.y[1] >= 0 && tri.y[1] < display.height()) ||
                              (tri.x[2] >= 0 && tri.x[2] < display.width() && tri.y[2] >= 0 && tri.y[2] < display.height());
            
            if (isOnScreen && onScreen < 3) {
                Serial.print("OnScreen Tri: (");
                Serial.print(tri.x[0]); Serial.print(","); Serial.print(tri.y[0]); Serial.print(") (");
                Serial.print(tri.x[1]); Serial.print(","); Serial.print(tri.y[1]); Serial.print(") (");
                Serial.print(tri.x[2]); Serial.print(","); Serial.print(tri.y[2]); Serial.println(")");
                onScreen++;
            }

            Raster::fillTriangle(display, 
                tri.x[0], tri.y[0],
                tri.x[1], tri.y[1],
                tri.x[2], tri.y[2],
                tri.color);
            
            // Simple wireframe effect: draw black outline
            display.drawLine(tri.x[0], tri.y[0], tri.x[1], tri.y[1], display::Color::Black());
            display.drawLine(tri.x[1], tri.y[1], tri.x[2], tri.y[2], display::Color::Black());
            display.drawLine(tri.x[2], tri.y[2], tri.x[0], tri.y[0], display::Color::Black());
            
            rendered++;
        }
    }
    Serial.print("Rendered triangles: ");
    Serial.println(rendered);
    Serial.println("--- Render Complete ---");
}

void Renderer::renderTestScene() {
    // Camera positioned to look directly at cube
    // Cube is at (0, 0, 4.5), camera at (0, 0, -3) looking forward
    Camera cam(Vec3({0.0f, 0.0f, -3.0f}), Vec3({0.0f, 0.0f, 1.0f}), 70.0f, 1.5f, 0.1f, 100.0f);
    
    // Get matrices
    Mat4 view = cam.getViewMatrix();
    Mat4 proj = cam.getProjectionMatrix();
    Mat4 viewProj = proj * view;
    
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
    Vec3 frontVerts[4] = {{x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}};
    projectAndAddQuad(frontVerts, 0xF800, viewProj, cam); // Red
    
    // Back face (-Z)
    Vec3 backVerts[4] = {{x1, y0, z0}, {x0, y0, z0}, {x0, y1, z0}, {x1, y1, z0}};
    projectAndAddQuad(backVerts, 0x07E0, viewProj, cam); // Green
    
    // Right face (+X)
    Vec3 rightVerts[4] = {{x1, y0, z1}, {x1, y0, z0}, {x1, y1, z0}, {x1, y1, z1}};
    projectAndAddQuad(rightVerts, 0x001F, viewProj, cam); // Blue
    
    // Left face (-X)
    Vec3 leftVerts[4] = {{x0, y0, z0}, {x0, y0, z1}, {x0, y1, z1}, {x0, y1, z0}};
    projectAndAddQuad(leftVerts, 0xFFE0, viewProj, cam); // Yellow
    
    // Top face (+Y)
    Vec3 topVerts[4] = {{x0, y1, z1}, {x1, y1, z1}, {x1, y1, z0}, {x0, y1, z0}};
    projectAndAddQuad(topVerts, 0xF81F, viewProj, cam); // Magenta
    
    // Bottom face (-Y)
    Vec3 bottomVerts[4] = {{x0, y0, z0}, {x1, y0, z0}, {x1, y0, z1}, {x0, y0, z1}};
    projectAndAddQuad(bottomVerts, 0x07FF, viewProj, cam); // Cyan
    
    Serial.print("Face count: ");
    Serial.println(faceList.count());
    
    // Sort faces by depth
    faceList.sortByDepth();
    
    // Rasterize all faces
    for (int i = 0; i < faceList.count(); i++) {
        const Triangle& tri = faceList[i];
        if (tri.visible) {
            Raster::fillTriangle(display, 
                tri.x[0], tri.y[0],
                tri.x[1], tri.y[1],
                tri.x[2], tri.y[2],
                tri.color);
        }
    }
    
    Serial.println("=== renderTestScene complete ===");
}

Vec4 Renderer::multiplyMatrixVector(const Vec3& v, const Mat4& m) {
    Vec4 result;
    result[0] = v[0] * m(0, 0) + v[1] * m(1, 0) + v[2] * m(2, 0) + m(3, 0);
    result[1] = v[0] * m(0, 1) + v[1] * m(1, 1) + v[2] * m(2, 1) + m(3, 1);
    result[2] = v[0] * m(0, 2) + v[1] * m(1, 2) + v[2] * m(2, 2) + m(3, 2);
    result[3] = v[0] * m(0, 3) + v[1] * m(1, 3) + v[2] * m(2, 3) + m(3, 3);
    return result;
}

Vec4 Renderer::multiplyMatrixVector(const Vec4& v, const Mat4& m) {
    Vec4 result;
    result[0] = v[0] * m(0, 0) + v[1] * m(1, 0) + v[2] * m(2, 0) + v[3] * m(3, 0);
    result[1] = v[0] * m(0, 1) + v[1] * m(1, 1) + v[2] * m(2, 1) + v[3] * m(3, 1);
    result[2] = v[0] * m(0, 2) + v[1] * m(1, 2) + v[2] * m(2, 2) + v[3] * m(3, 2);
    result[3] = v[0] * m(0, 3) + v[1] * m(1, 3) + v[2] * m(2, 3) + v[3] * m(3, 3);
    return result;
}

Vec3 Renderer::computeNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
    // Compute edges
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
    
    // Cross product
    Vec3 normal;
    normal[0] = edge1[1] * edge2[2] - edge1[2] * edge2[1];
    normal[1] = edge1[2] * edge2[0] - edge1[0] * edge2[2];
    normal[2] = edge1[0] * edge2[1] - edge1[1] * edge2[0];
    
    // Normalize
    float len = sqrtf(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
    if (len > 0.0001f) {
        normal[0] /= len;
        normal[1] /= len;
        normal[2] /= len;
    }
    
    return normal;
}

bool Renderer::shouldCullFace(const Vec3& normal, const Vec3& faceCenter, const Vec3& cameraPos) {
    // Vector from face to camera
    Vec3 viewDir;
    viewDir[0] = cameraPos[0] - faceCenter[0];
    viewDir[1] = cameraPos[1] - faceCenter[1];
    viewDir[2] = cameraPos[2] - faceCenter[2];
    
    // Dot product
    float dot = normal[0]*viewDir[0] + normal[1]*viewDir[1] + normal[2]*viewDir[2];
    
    // If dot < 0, face is pointing away from camera
    return dot <= 0.0f;
}

bool Renderer::projectVertex(const Vec3& worldPos, const Mat4& viewProj, int16_t& outX, int16_t& outY, float& outZ) {
    // Transform to clip space
    Vec4 clipPos = multiplyMatrixVector(worldPos, viewProj);
    
    // Check if behind near plane
    if (clipPos[3] < 0.1f) {
        return false;
    }
    
    // Perspective divide
    float invW = 1.0f / clipPos[3];
    float ndcX = clipPos[0] * invW;
    float ndcY = clipPos[1] * invW;
    float ndcZ = clipPos[2] * invW;
    
    // Debug center block projection (approximate check)
    if (abs(worldPos[0] - 5.0f) < 0.1f && abs(worldPos[2]) < 0.1f) {
        Serial.print("Proj (5,y,0): NDC=(");
        Serial.print(ndcX); Serial.print(",");
        Serial.print(ndcY); Serial.print(") W=");
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
    outZ = clipPos[3]; // Use W for depth (distance from camera)
    
    return true;
}

void Renderer::projectAndAddQuad(
    const Vec3 verts[4],
    uint16_t color,
    const Mat4& viewProj,
    const Camera& cam
) {
    // Compute face center
    Vec3 center;
    center[0] = (verts[0][0] + verts[1][0] + verts[2][0] + verts[3][0]) * 0.25f;
    center[1] = (verts[0][1] + verts[1][1] + verts[2][1] + verts[3][1]) * 0.25f;
    center[2] = (verts[0][2] + verts[1][2] + verts[2][2] + verts[3][2]) * 0.25f;
    
    // Compute normal
    Vec3 normal = computeNormal(verts[0], verts[1], verts[2]);
    
    // Backface culling
    if (shouldCullFace(normal, center, cam.position)) {
        return;
    }
    
    // Project vertices
    int16_t screenX[4], screenY[4];
    float depth[4];
    int validVerts = 0;
    
    for (int i = 0; i < 4; i++) {
        if (projectVertex(verts[i], viewProj, screenX[i], screenY[i], depth[i])) {
            validVerts++;
        }
    }
    
    // If any vertices are behind camera, skip (simplified clipping)
    if (validVerts < 4) {
        return;
    }
    
    // Split quad into two triangles: 0-1-2 and 0-2-3
    float avgDepth1 = (depth[0] + depth[1] + depth[2]) / 3.0f;
    float avgDepth2 = (depth[0] + depth[2] + depth[3]) / 3.0f;
    
    // Triangle 1: 0-1-2
    Triangle tri1;
    tri1.x[0] = screenX[0]; tri1.y[0] = screenY[0];
    tri1.x[1] = screenX[1]; tri1.y[1] = screenY[1];
    tri1.x[2] = screenX[2]; tri1.y[2] = screenY[2];
    tri1.depth = avgDepth1;
    tri1.color = color;
    tri1.visible = true;
    faceList.add(tri1);
    
    // Triangle 2: 0-2-3
    Triangle tri2;
    tri2.x[0] = screenX[0]; tri2.y[0] = screenY[0];
    tri2.x[1] = screenX[2]; tri2.y[1] = screenY[2];
    tri2.x[2] = screenX[3]; tri2.y[2] = screenY[3];
    tri2.depth = avgDepth2;
    tri2.color = color;
    tri2.visible = true;
    faceList.add(tri2);
}

void Renderer::extractFaces(const World& world, const Camera& cam, const Mat4& viewProj) {
    // Voxlap-style face extraction: only emit faces adjacent to air
    for (int x = 0; x < World::WIDTH; x++) {
        for (int y = 0; y < World::HEIGHT; y++) {
            for (int z = 0; z < World::DEPTH; z++) {
                uint8_t block = world.getBlock(x, y, z);
                if (block == AIR) continue;
                
                // Apply offset to center the world
                // World is 0-10. Center X at 0 (subtract 5)
                // Move Y down so camera is "above" (subtract 2)
                // Move Z forward but closer (add 2) - this makes blocks look bigger
                float worldX = (float)x - 5.0f;
                float worldY = (float)y - 2.0f; 
                float worldZ = (float)z + 2.0f;
                
                // Define vertices relative to block origin
                float x0 = worldX, x1 = worldX + 1.0f;
                float y0 = worldY, y1 = worldY + 1.0f;
                float z0 = worldZ, z1 = worldZ + 1.0f;
                
                // Check 6 neighbors
                struct { int dx, dy, dz; } dirs[] = {
                    {0, 0, 1}, {0, 0, -1},  // Front, Back
                    {1, 0, 0}, {-1, 0, 0},  // Right, Left
                    {0, 1, 0}, {0, -1, 0}   // Top, Bottom
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
                        verts[0] = {x0, y0, z1}; verts[1] = {x1, y0, z1};
                        verts[2] = {x1, y1, z1}; verts[3] = {x0, y1, z1};
                    } else if (d.dz == -1) { // Back (-Z)
                        verts[0] = {x1, y0, z0}; verts[1] = {x0, y0, z0};
                        verts[2] = {x0, y1, z0}; verts[3] = {x1, y1, z0};
                    } else if (d.dx == 1) { // Right (+X)
                        verts[0] = {x1, y0, z1}; verts[1] = {x1, y0, z0};
                        verts[2] = {x1, y1, z0}; verts[3] = {x1, y1, z1};
                    } else if (d.dx == -1) { // Left (-X)
                        verts[0] = {x0, y0, z0}; verts[1] = {x0, y0, z1};
                        verts[2] = {x0, y1, z1}; verts[3] = {x0, y1, z0};
                    } else if (d.dy == 1) { // Top (+Y)
                        verts[0] = {x0, y1, z1}; verts[1] = {x1, y1, z1};
                        verts[2] = {x1, y1, z0}; verts[3] = {x0, y1, z0};
                    } else { // Bottom (-Y)
                        verts[0] = {x0, y0, z0}; verts[1] = {x1, y0, z0};
                        verts[2] = {x1, y0, z1}; verts[3] = {x0, y0, z1};
                    }
                    
                    // Assign color based on block type
                    if (block == DIRT) {
                        color = 0x79E0; // Brown
                    } else if (block == GRASS) {
                        if (d.dy == 1) color = 0x07E0; // Green top
                        else if (d.dy == -1) color = 0x79E0; // Brown bottom
                        else color = 0x7BE0; // Green-brown side
                    } else if (block == STONE) {
                        color = 0x8410; // Gray
                    } else {
                        color = 0xFFFF; // White
                    }
                    
                    // Project and add quad
                    projectAndAddQuad(verts, color, viewProj, cam);
                }
            }
        }
    }
}
