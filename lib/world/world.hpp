#pragma once

#include <blocks.hpp>
#include <stdint.h>

class World {
public:
    static const int WIDTH = 10;
    static const int HEIGHT = 10;
    static const int DEPTH = 10;

    Block blocks[WIDTH][HEIGHT][DEPTH];

    World();

    bool inBounds(int x, int y, int z) const;
    Block getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, Block id);
    bool isSolid(Block block) const;
    void fillTestWorld();
};