#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "World.h"
#include "Player.h"

class GameState {
public:
    World world;
    Player player;

    GameState();

    void update(float dt);
};

#endif
