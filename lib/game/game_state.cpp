#include "game_state.hpp"
#include <algebra.hpp>
#include <cmath>
#include <general.hpp>

GameState::GameState(Renderer& renderer, Player& player)
    : player(player), renderer(renderer) {
    world.fillTestWorld();
}

void GameState::update(float dt) {
    player.updatePhysics(world, dt);
    render();
}

void GameState::render() {
    renderer.render(world, player.position, player.getOrientation());
}

static algebra::Vector<3> forwardDirection(const Player& player) {
    algebra::Vector<3> dir = algebra::Vector<3>({ 0.0f, 0.0f, 1.0f });
    dir = algebra::rotateVector(player.getOrientation(), dir);
    const float len =
        sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
    if (len > 1e-4f) {
        dir[0] /= len;
        dir[1] /= len;
        dir[2] /= len;
    }
    return dir;
}

bool GameState::placeStoneForward(const Player& player, float maxDist) {
    const algebra::Vector<3> origin =
        player.position + algebra::Vector<3>({ 0.0f, player.height * 0.9f, 0.0f });
    const algebra::Vector<3> dir = forwardDirection(player);
    const float step = 0.1f;

    bool hasPrev = false;
    int prevX = 0, prevY = 0, prevZ = 0;

    for (float d = 0.0f; d <= maxDist; d += step) {
        const algebra::Vector<3> p = origin + dir * d;
        const int bx = static_cast<int>(floorf(p[0]));
        const int by = static_cast<int>(floorf(p[1]));
        const int bz = static_cast<int>(floorf(p[2]));
        if (!world.inBounds(bx, by, bz)) { continue; }

        if (world.getBlock(bx, by, bz) != AIR) {
            if (hasPrev && world.inBounds(prevX, prevY, prevZ)) {
                world.setBlock(prevX, prevY, prevZ, STONE);
                println("[Main] Placed STONE at:", prevX, ",", prevY, ",", prevZ);
                return true;
            }
            return false;
        } else {
            hasPrev = true;
            prevX = bx;
            prevY = by;
            prevZ = bz;
        }
    }
    return false;
}

bool GameState::removeBlockForward(const Player& player, float maxDist) {
    const algebra::Vector<3> origin =
        player.position + algebra::Vector<3>({ 0.0f, player.height * 0.9f, 0.0f });
    const algebra::Vector<3> dir = forwardDirection(player);
    const float step = 0.1f;

    for (float d = 0.0f; d <= maxDist; d += step) {
        const algebra::Vector<3> p = origin + dir * d;
        const int bx = static_cast<int>(floorf(p[0]));
        const int by = static_cast<int>(floorf(p[1]));
        const int bz = static_cast<int>(floorf(p[2]));
        if (!world.inBounds(bx, by, bz)) { continue; }

        if (world.getBlock(bx, by, bz) != AIR) {
            world.setBlock(bx, by, bz, AIR);
            println("[Main] Removed block at:", bx, ",", by, ",", bz);
            return true;
        }
    }
    return false;
}
