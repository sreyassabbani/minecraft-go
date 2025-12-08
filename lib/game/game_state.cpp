#include "game_state.hpp"

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
