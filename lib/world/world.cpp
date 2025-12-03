#include <world.hpp>

World::World() {
    // Initialize with AIR
    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            for (int z = 0; z < DEPTH; ++z) { blocks[x][y][z] = AIR; }
        }
    }
}

bool World::inBounds(int x, int y, int z) const {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT && z >= 0 && z < DEPTH;
}

Block World::getBlock(int x, int y, int z) const {
    if (!inBounds(x, y, z)) { return AIR; }
    return blocks[x][y][z];
}

void World::setBlock(int x, int y, int z, Block id) {
    if (inBounds(x, y, z)) { blocks[x][y][z] = id; }
}

bool World::isSolid(Block block) const { return block != AIR; }

void World::fillTestWorld() {
    // Floor
    // Floor (checkerboard for visibility)
    for (int x = 0; x < WIDTH; ++x) {
        for (int z = 0; z < DEPTH; ++z) {
            if ((x + z) % 2 == 0) setBlock(x, 0, z, GRASS);
            else setBlock(x, 0, z, STONE);
        }
    }

    // Some random pillars/blocks for testing
    setBlock(5, 1, 5, DIRT);
    setBlock(5, 2, 5, GRASS);

    setBlock(2, 1, 2, STONE);
    setBlock(2, 2, 2, STONE);
    setBlock(2, 3, 2, STONE);
}
