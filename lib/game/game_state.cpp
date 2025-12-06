#include "game_state.hpp"

GameState::GameState() {
    world.fillTestWorld();
    // Player is initialized in its own constructor
}

void GameState::update(float dt) {
    player.updatePhysics(world, dt); 
    
    
    player.move();
}
