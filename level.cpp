#include "level.h"
#include "player.h"
#include "bullet.h" 
#include <collision.h>

Level loadLevel(std::vector<std::string> map) {
    Level level;
    level.tiles = map;
    level.rows  = map.size();
    level.cols  = map.empty() ? 0 : map[0].size();
    return level;
}

bool tileSolid(const Level& level, int col, int row) {
    if(row < 0 || row >= level.rows) return false;
    if(col < 0 || col >= level.cols) return false;
    return level.tiles[row][col] == '1';
}

void drawLevel(const Level& level) {
    for(int row = 0; row < level.rows; row++) {
        for(int col = 0; col < level.cols; col++) {
            if(tileSolid(level, col, row)) {
                Vec2 pos(col * TILE_SIZE, row * TILE_SIZE);
                drawRect(pos, Vec2(TILE_SIZE, TILE_SIZE), Color::red);
            }
        }
    }
}

void resolvePlayerLevel(Player& player, const Level& level) {
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

void resolveBulletLevel(Bullet& bullet, const Level& level) {
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