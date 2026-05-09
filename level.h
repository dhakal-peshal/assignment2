#ifndef level_h_
#define level_h_

#include <engine.h>
#include <vector>
#include <string>
#include "player.h"

const int TILE_SIZE = 32;

struct Level {
    std::vector<std::string> tiles;
    int rows, cols;
};

Level loadLevel(std::vector<std::string> map);
bool tileSolid(const Level& level, int col, int row);
void drawLevel(const Level& level);
void colPlayerLevel(Player& player, const Level& level);

#endif