#include "game_state.hpp"

GameState::GameState(Renderer& renderer, Player& player)
    : player(player), renderer(renderer) {
    world.fillTestWorld();
}

void GameState::update(float dt) {
    render();

    player.updatePhysics(world, dt);
    player.move();
}

void GameState::render() {
    renderer.render(world, player.position, player.getOrientation());
}
