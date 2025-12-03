#pragma once

#include <player.hpp>
#include <world.hpp>

class GameState {
public:
    World world;
    Player player;

    GameState();

    void update(float dt);
};