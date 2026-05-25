#include "level.h"
#include "player.h"
#include "bullet.h" 
#include "villain.h"
#include <fstream>
#include <collision.h>
using json = nlohmann::json;

Texture bg;

World loadWorld(const std::string& path, Texture spritesheet) {
    std::ifstream file(path);
    json data = json::parse(file);

    World world;
    world.currentLevel = 0;

    for(auto& lvl : data["levels"]) {
        LevelData level;
        // load level textures
        level.groundTexture = subTexture(spritesheet, Rect{0, 0, 8, 8});
        level.brickTexture = subTexture(spritesheet, Rect{8, 0, 8, 8});
        level.woodTexture = subTexture(spritesheet, Rect{16, 0, 8, 8});
        level.sheetTexture = subTexture(spritesheet, Rect{24, 0, 8, 8});
        // load level data
        level.id = lvl["id"];
        level.bg = lvl["bg"];
        level.rows = lvl["tiles"].size();
        level.cols = lvl["tiles"][0].get<std::string>().size();
        level.neighbourLeft  = lvl["neighbours"]["left"];
        level.neighbourRight = lvl["neighbours"]["right"];
        level.neighbourUp    = lvl["neighbours"]["up"];
        level.neighbourDown  = lvl["neighbours"]["down"];

        // background loading
        if (level.bg == 0) level.background = loadTexture("assets/bg_out_l.png");
        else if (level.bg == 1) level.background = loadTexture("assets/bg_wood.png");
        else if (level.bg == 2) level.background = loadTexture("assets/bg_out_down.png");
        else if (level.bg == 3) level.background = loadTexture("assets/bg_brick.png");
        else if (level.bg == 4) level.background = loadTexture("assets/bg_out_up.png");
        else if (level.bg == 5) level.background = loadTexture("assets/bg_out_top.png");
        else if (level.bg == 6) level.background = loadTexture("assets/bg_out_r.png");

        // level tile allocation
        for(auto& row : lvl["tiles"])
            level.tiles.push_back(row.get<std::string>());
        world.levels.push_back(level);

        SDL_SetTextureScaleMode(level.background.texture, SDL_SCALEMODE_NEAREST); // .texture
    }
    return world;
}

LevelData& currentLevel(World& world) {
    return world.levels[world.currentLevel];
}
// check if tile should apply collision
bool tileSolid(const LevelData& level, int col, int row) {
    if(row < 0 || row >= level.rows) return false;
    if(col < 0 || col >= level.cols) return false;
    // slightly scuffed implementation, 0 for air tiles and other numbers for spawn tiles
    return level.tiles[row][col] != '0' && level.tiles[row][col] != '8';
}

int checkLevelTransition(Player& player, const LevelData& level) {
    float worldWidth  = level.cols * TILE_SIZE;
    float worldHeight = level.rows * TILE_SIZE;

    if(player.transform.localPosition.x < 0) return level.neighbourLeft;
    if(player.transform.localPosition.x > worldWidth) return level.neighbourRight;
    if(player.transform.localPosition.y < 0) return level.neighbourUp;
    if(player.transform.localPosition.y > worldHeight) return level.neighbourDown;

    return -1;
}
// function to handle enemy spawners in place of tiles
void spawnLevelEntities(LevelData &level, std::vector<Villain> &villains, Texture enemySprites) {
    level.pickups = level.originalPickups;
    for(int row = 0; row < level.rows; row++) {
        for(int col = 0; col < level.cols; col++) {
            char tile = level.tiles[row][col];
            if(tile == '8') { // enemy spawning
                Villain v;
                Vec2 worldPos(col * TILE_SIZE, row * TILE_SIZE);
                initVillain(v, worldPos, enemySprites);
                villains.push_back(v);
            } else if(tile == 's' || tile == 'b') { // item pickups
                PickupData pickup;
                pickup.type = tile;
                pickup.col = col;
                pickup.row = row;
                pickup.active = true;
                level.pickups.push_back(pickup);
                level.tiles[row][col] = '0';
            }
        }
    }
    level.originalPickups = level.pickups;
}

void wrapPlayerPosition(Player& player, const LevelData& level) {
    float worldWidth  = level.cols * TILE_SIZE;
    float worldHeight = level.rows * TILE_SIZE;

    if(player.transform.localPosition.x < 0)           player.transform.localPosition.x = worldWidth  - PLAYER_SIZE_X - 1;
    if(player.transform.localPosition.x > worldWidth)  player.transform.localPosition.x = 1;
    if(player.transform.localPosition.y < 0)           player.transform.localPosition.y = worldHeight - PLAYER_SIZE_Y - 1;
    if(player.transform.localPosition.y > worldHeight) player.transform.localPosition.y = 1;
}

void resolvePlayerLevel(Player& player, const LevelData& level) {
    Vec2 pSize(PLAYER_SIZE_X, PLAYER_SIZE_Y);
    Vec2& pos = player.transform.localPosition;  // reference so changes apply directly

    int left   = (int)(pos.x) / TILE_SIZE;
    int right  = (int)(pos.x + pSize.x) / TILE_SIZE;
    int top    = (int)(pos.y) / TILE_SIZE;
    int bottom = (int)(pos.y + pSize.y) / TILE_SIZE;

    player.grounded = false;  // reset each frame, let collision set it back
    player.onLeftWall = false;
    player.onRightWall = false;

    for(int row = top; row <= bottom; row++) {
        for(int col = left; col <= right; col++) {
            if(!tileSolid(level, col, row)) continue;

            Vec2 tilePos(col * TILE_SIZE, row * TILE_SIZE);
            Vec2 tileSize(TILE_SIZE, TILE_SIZE);

            if(!collision(pos, pSize, tilePos, tileSize)) continue;

            float overlapLeft   = (tilePos.x + TILE_SIZE) - pos.x;
            float overlapRight  = (pos.x + pSize.x) - tilePos.x;
            float overlapTop    = (tilePos.y + TILE_SIZE) - pos.y;
            float overlapBottom = (pos.y + pSize.y) - tilePos.y;

            float minX = std::min(overlapLeft, overlapRight);
            float minY = std::min(overlapTop,  overlapBottom);

            if(minX < minY) {
                if(overlapLeft < overlapRight) {
                    pos.x += overlapLeft;
                    player.onLeftWall = true;
                }
                else {
                    pos.x -= overlapRight;
                    player.onRightWall = true;
                }
                player.vel.x = 0;
            } else {
                if(overlapTop < overlapBottom) {
                    pos.y += overlapTop;
                    player.vel.y = 0;
                } else {
                    pos.y -= overlapBottom;
                    player.vel.y = 0;
                    player.grounded = true;
                }
            }
        }
    }
}

void resolveBulletLevel(Bullet& bullet, const LevelData& level) {
    if(!bullet.active) return;

    Vec2& pos = bullet.transform.localPosition;
    Vec2 halfSize = bullet.size / 2;
    Vec2 topLeft = pos - halfSize;  // bullet origin is centred

    int left   = (int)(topLeft.x) / TILE_SIZE;
    int right  = (int)(topLeft.x + bullet.size.x) / TILE_SIZE;
    int top    = (int)(topLeft.y) / TILE_SIZE;
    int bottom = (int)(topLeft.y + bullet.size.y) / TILE_SIZE;

    for(int row = top; row <= bottom; row++) {
        for(int col = left; col <= right; col++) {
            if(!tileSolid(level, col, row)) continue;

            Vec2 tilePos(col * TILE_SIZE, row * TILE_SIZE);
            Vec2 tileSize(TILE_SIZE, TILE_SIZE);

            if(collision(topLeft, bullet.size, tilePos, tileSize)) {
                bullet.active = false;
                return;  // no need to check further tiles
            }
        }
    }
}

void resolvePickups(LevelData &level, Player &player, bool &showTextBox, char &textBoxPickup, float &textBoxTimer, float textBoxDuration) {
    Vec2 playerPos  = player.transform.localPosition;
    Vec2 playerSize = Vec2(PLAYER_SIZE_X, PLAYER_SIZE_Y);

    for(PickupData &pickup : level.pickups) {
        if(!pickup.active) continue;

        Vec2 pickupPos(pickup.col * TILE_SIZE, pickup.row * TILE_SIZE);
        Vec2 pickupSize(TILE_SIZE, TILE_SIZE);

        if(collision(playerPos, playerSize, pickupPos, pickupSize)) {
            pickup.active = false;

            if(pickup.type == 's') player.hasShotgun = true;
            if(pickup.type == 'b') player.hasBoot = true;

            showTextBox = true;
            textBoxPickup = pickup.type;
            textBoxTimer = textBoxDuration;
        }
    }
}

void drawLevel(const LevelData& level) {
    drawTexture(level.background, Vec2(0,0), Vec2(320, 180)*4);
    for(int row = 0; row < level.rows; row++) {
        for(int col = 0; col < level.cols; col++) {
            if(tileSolid(level, col, row)) {
                char tile = level.tiles[row][col];
                Vec2 pos(col * TILE_SIZE, row * TILE_SIZE);
                //drawRect(pos, Vec2(TILE_SIZE, TILE_SIZE), Color::red);
                switch(tile) {
                    case '1': drawTexture(level.groundTexture, pos, Vec2(32, 32)); break;
                    case '2': drawTexture(level.brickTexture,   pos, Vec2(32, 32)); break;
                    case '3': drawTexture(level.woodTexture,  pos, Vec2(32, 32)); break;
                    case '4': drawTexture(level.sheetTexture,  pos, Vec2(32, 32)); break;
                    case 'a': break; // air/invisible tile
                    default:  drawTexture(level.groundTexture, pos, Vec2(32, 32)); break;
                }
            }
        }
    }
}