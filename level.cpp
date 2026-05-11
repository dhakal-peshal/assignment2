#include "level.h"
#include "player.h"
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

void colPlayerLevel(Player& player, const Level& level) {
    Vec2 pSize(PLAYER_SIZE_X, PLAYER_SIZE_Y);

    // find what tiles the player overlaps with
    int left   = (int)(player.pos.x) / TILE_SIZE;
    int right  = (int)(player.pos.x + pSize.x) / TILE_SIZE;
    int top    = (int)(player.pos.y) / TILE_SIZE;
    int bottom = (int)(player.pos.y + pSize.y) / TILE_SIZE;

    for(int row = top; row <= bottom; row++) {
        for(int col = left; col <= right; col++) {
            if(!tileSolid(level, col, row)) continue;

            Vec2 tilePos(col * TILE_SIZE, row * TILE_SIZE);
            Vec2 tileSize(TILE_SIZE, TILE_SIZE);

            if(!collision(player.pos, pSize, tilePos, tileSize)) continue;

            // resolve by finding the shallowest overlap axis
            float overlapLeft   = (tilePos.x + TILE_SIZE) - player.pos.x;
            float overlapRight  = (player.pos.x + pSize.x) - tilePos.x;
            float overlapTop    = (tilePos.y + TILE_SIZE) - player.pos.y;
            float overlapBottom = (player.pos.y + pSize.y) - tilePos.y;

            float minX = std::min(overlapLeft, overlapRight);
            float minY = std::min(overlapTop,  overlapBottom);

            if(minX < minY) {
                // push player away if overlapping, think this is slightly buggy (player sometimes jitters/slows)
                if(overlapLeft < overlapRight)
                    player.pos.x += overlapLeft;
                else
                    player.pos.x -= overlapRight;
                player.vel.x = 0;
            } else {
                if(overlapTop < overlapBottom) {
                    player.pos.y += overlapTop;
                    player.vel.y = 0;
                } else {
                    player.pos.y -= overlapBottom;
                    player.vel.y = 0;
                    player.grounded = true;
                }
            }
        }
    }
}