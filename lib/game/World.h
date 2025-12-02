#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

enum BlockType {
    AIR = 0,
    DIRT = 1,
    GRASS = 2,
    STONE = 3
};

class World {
public:
    static const int WIDTH = 10;
    static const int HEIGHT = 10;
    static const int DEPTH = 10;

    uint8_t blocks[WIDTH][HEIGHT][DEPTH];

    World();

    bool inBounds(int x, int y, int z) const;
    uint8_t getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, uint8_t id);
    bool isSolid(uint8_t block) const;
    void fillTestWorld();
};

#endif
