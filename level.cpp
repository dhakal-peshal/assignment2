#include "level.h"
#include "player.h"
#include "bullet.h" 
#include <fstream>
#include <collision.h>
using json = nlohmann::json;

World loadWorld(const std::string& path, Texture spritesheet) {
    std::ifstream file(path);
    json data = json::parse(file);

    World world;
    world.currentLevel = 0;

    for(auto& lvl : data["levels"]) {
        LevelData level;
        // load level textures
        level.groundTexture = subTexture(spritesheet, Rect{0, 0, 16, 16});
        level.brickTexture = subTexture(spritesheet, Rect{16, 0, 16, 16});
        level.woodTexture = subTexture(spritesheet, Rect{32, 0, 16, 16});
        level.sheetTexture = subTexture(spritesheet, Rect{48, 0, 16, 16});
        // load level data
        level.id   = lvl["id"];
        level.rows = lvl["tiles"].size();
        level.cols = lvl["tiles"][0].get<std::string>().size();
        level.neighbourLeft  = lvl["neighbours"]["left"];
        level.neighbourRight = lvl["neighbours"]["right"];
        level.neighbourUp    = lvl["neighbours"]["up"];
        level.neighbourDown  = lvl["neighbours"]["down"];

        for(auto& row : lvl["tiles"])
            level.tiles.push_back(row.get<std::string>());

        world.levels.push_back(level);
    }
    return world;
}

LevelData& currentLevel(World& world) {
    return world.levels[world.currentLevel];
}

bool tileSolid(const LevelData& level, int col, int row) {
    if(row < 0 || row >= level.rows) return false;
    if(col < 0 || col >= level.cols) return false;
    return level.tiles[row][col] != '0';
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
                if(overlapLeft < overlapRight) pos.x += overlapLeft;
                else                           pos.x -= overlapRight;
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

void drawLevel(const LevelData& level) {
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
                    default:  drawTexture(level.groundTexture, pos, Vec2(32, 32)); break;
                }
            }
        }
    }
}