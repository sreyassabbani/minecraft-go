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

    // Forward-facing interactions
    bool placeStoneForward(const Player& player, float maxDist);
    bool removeBlockForward(const Player& player, float maxDist);
};
