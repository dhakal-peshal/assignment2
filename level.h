#ifndef level_h_
#define level_h_

#include <engine.h>
#include <vector>
#include <string>
#include "player.h"
#include "bullet.h"
#include "nlohmann/json.hpp"

const int TILE_SIZE = 32;

struct LevelData {
    int id;
    std::vector<std::string> tiles;
    int rows, cols;
    int neighbourLeft, neighbourRight, neighbourUp, neighbourDown;
    Texture groundTexture, brickTexture, woodTexture, sheetTexture;
};

struct World {
    std::vector<LevelData> levels;
    int currentLevel;
};

World loadWorld(const std::string& path, Texture texture);
LevelData& currentLevel(World& world);
bool tileSolid(const LevelData& level, int col, int row);
void drawLevel(const LevelData& level);
int checkLevelTransition(Player& player, const LevelData& level);
void wrapPlayerPosition(Player& player, const LevelData& level);

void resolvePlayerLevel(Player& player, const LevelData& level);
void resolveBulletLevel(Bullet& bullet, const LevelData& level);

#endif