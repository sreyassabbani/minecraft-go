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

bool GameState::placeStoneForward(Player& player, float maxDist) {
    (void)player;
    (void)maxDist;

    const int bx = 3, by = 1, bz = 3;
    if (!world.inBounds(bx, by, bz)) {
        println("[Main] Place failed: 3,1,3 out of bounds");
        return false;
    }

    world.setBlock(bx, by, bz, STONE);
    println("[Main] Placed STONE at (3,1,3)");
    return true;
}

bool GameState::removeBlockForward(Player& player, float maxDist) {
    (void)player;
    (void)maxDist;

    const int bx = 3, by = 1, bz = 3;
    if (!world.inBounds(bx, by, bz)) {
        println("[Main] Destroy failed: 3,1,3 out of bounds");
        return false;
    }

    world.setBlock(bx, by, bz, AIR);
    println("[Main] Removed block at (3,1,3)");
    return true;
}
