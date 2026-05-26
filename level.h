#ifndef level_h_
#define level_h_

#include <engine.h>
#include <vector>
#include <string>
#include "player.h"
#include "bullet.h"
#include "nlohmann/json.hpp"

const int TILE_SIZE = 32;

struct Villain; // forward decloration to avoid circular dependencies

struct PickupData {
    char type; 
    int col, row;
    bool active;
};

struct LevelData {
    int id, bg;
    std::vector<std::string> tiles;
    int rows, cols;
    int neighbourLeft, neighbourRight, neighbourUp, neighbourDown;
    Texture groundTexture, brickTexture, woodTexture, sheetTexture, background;
    //Texture spikeTexture;
    //Texture healthTexture;

    std::vector<PickupData> pickups;
    std::vector<PickupData> originalPickups;
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
void spawnLevelEntities(LevelData& level, std::vector<Villain>& villains, Texture enemySprites);
void wrapPlayerPosition(Player& player, const LevelData& level);

void resolvePlayerLevel(Player& player, const LevelData& level);
void resolveBulletLevel(Bullet& bullet, const LevelData& level);
void resolvePickups(LevelData &level, Player &player, bool &showTextBox, char &textBoxPickup, float &textBoxTimer, float textBoxDuration);

#endif