#pragma once

#include <player.hpp>
#include <renderer.hpp>
#include <world.hpp>

class GameState {
public:
    World world;
    Player& player;
    Renderer& renderer;

    GameState(Renderer& renderer, Player& player);

    void update(float dt);
    void render();
};
