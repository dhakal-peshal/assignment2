#include "villain.h"
#include "player.h"
#include "collision.h"
#include <cmath>
#include <algorithm>



float sign(float v) { return (v > 0.f) ? 1.f : (v < 0.f) ? -1.f : 0.f; }

void resolveVillainLevel(Villain &v, const LevelData &level) {
    v.grounded = false;

    for (int row = 0; row < level.rows; row++) {
        for (int col = 0; col < level.cols; col++) {
            if (!tileSolid(level, col, row)) continue;

            Vec2 tilePos(col * TILE_SIZE, row * TILE_SIZE);
            Vec2 tileSize(TILE_SIZE, TILE_SIZE);

            if (!collision(v.transform.localPosition, v.size, tilePos, tileSize)) continue;

            float overlapLeft  = (v.transform.localPosition.x + v.size.x) - tilePos.x;
            float overlapRight = (tilePos.x + tileSize.x) - v.transform.localPosition.x;
            float overlapTop   = (v.transform.localPosition.y + v.size.y) - tilePos.y;
            float overlapDown  = (tilePos.y + tileSize.y) - v.transform.localPosition.y;

            float minX = (overlapLeft < overlapRight) ? overlapLeft  : -overlapRight;
            float minY = (overlapTop  < overlapDown)  ? overlapTop   : -overlapDown;

            if (std::abs(minX) < std::abs(minY)) {
                v.transform.localPosition.x -= minX;
                v.vel.x = 0;
            } else {
                if (minY > 0) {           
                    v.grounded = true;
                    v.vel.y = 0;
                }
                v.transform.localPosition.y -= minY;
            }
        }
    }
}

bool edgeAhead(Villain &v, const LevelData &level) {
    float footX = v.transform.localPosition.x + (v.facingRight ? v.size.x + 2.f : -2.f);
    float footY = v.transform.localPosition.y + v.size.y + 4.f;

    int col = (int)(footX / TILE_SIZE);
    int row = (int)(footY / TILE_SIZE);

    if (col < 0 || col >= level.cols || row < 0 || row >= level.rows) return true;
    return !tileSolid(level, col, row);
}


void initVillain(Villain &v, Vec2 startPos, Texture spritesheet) {
    v.transform.localPosition = startPos;
    v.transform.localAngle    = 0.f;
    v.transform.localScale    = Vec2(1, 1);
    v.transform.parent        = nullptr;

    v.vel        = Vec2(0, 0);
    v.size       = Vec2(VILLAIN_W, VILLAIN_H);
    v.hp         = 4;
    v.maxHp      = 4;
    v.grounded   = false;
    v.facingRight = true;

    v.state       = VillainState::PATROL;
    v.patrolDir   = 1.f;
    v.patrolTimer = 2.f;
    v.hurtTimer   = 0.f;
    v.flashTimer  = 0.f;

    v.chaseRange     = 220.f;
    v.attackRange    = 30.f;
    v.attackCooldown = 1.0f;
    v.attackTimer    = 0.f;

    // idle animation
    for(int i = 0; i < 2; i++)
        v.idle.frames.push_back(subTexture(spritesheet, Rect{i * 16.0f, 0, 16, 16}));
    v.idle.no_frames = v.idle.frames.size();
    v.idle.duration  = 0.8f;
    v.idle.loop      = true;

    // walking animation
    for(int i = 2; i < 6; i++)
        v.walk.frames.push_back(subTexture(spritesheet, Rect{i * 16.0f, 0, 16, 16}));
    v.walk.no_frames = v.walk.frames.size();
    v.walk.duration  = 0.4f;
    v.walk.loop      = true;

    // jump
    v.jump = subTexture(spritesheet, Rect{6 * 16.0f, 0, 16, 16});

    // knife sprite
    v.knifeTexture = subTexture(spritesheet, Rect{112.0f, 8, 8, 8});
    v.transform.addChild(&v.knifeTransform);
    v.knifeTransform.localPosition = Vec2(VILLAIN_W / 2, VILLAIN_H / 2);

    v.animStart = getTimeInSeconds();
    v.frame = 0;
}


void updateVillain(Villain &v, Vec2 playerPos, int &playerHp, float dt, const LevelData &level) {
    if (v.state == VillainState::DEAD) return;

    const float gravity     = 900.f;
    const float patrolSpeed = 80.f;
    const float chaseSpeed  = 150.f;

    if (v.hurtTimer  > 0.f) v.hurtTimer  -= dt;
    if (v.flashTimer > 0.f) v.flashTimer -= dt;
    if (v.attackTimer > 0.f) v.attackTimer -= dt;

    Vec2  toPlayer  = playerPos - v.transform.localPosition;
    float distToPlayer = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (v.state == VillainState::HURT) {
        if (v.hurtTimer <= 0.f)
            v.state = (distToPlayer < v.chaseRange) ? VillainState::CHASE : VillainState::PATROL;
    } else if (v.state == VillainState::PATROL) {
        if (distToPlayer < v.chaseRange)
            v.state = VillainState::CHASE;
    } else if (v.state == VillainState::CHASE) {
        if (distToPlayer > v.chaseRange * 1.3f)  
            v.state = VillainState::PATROL;
    }

    float targetVelX = 0.f;

    if (v.state == VillainState::PATROL) {
        v.patrolTimer -= dt;
        bool flip = (v.patrolTimer <= 0.f) || (v.grounded && edgeAhead(v, level));
        if (flip) {
            v.patrolDir   = -v.patrolDir;
            v.patrolTimer = 2.5f;
        }
        targetVelX = v.patrolDir * patrolSpeed;

    } else if (v.state == VillainState::CHASE) {
        if (distToPlayer > v.attackRange)
            targetVelX = sign(toPlayer.x) * chaseSpeed;

        // Attack player when close enough
        if (distToPlayer <= v.attackRange && v.attackTimer <= 0.f) {
            playerHp --;
            v.attackTimer = v.attackCooldown;
        }
    }

    v.vel.x      = targetVelX;
    v.facingRight = (v.vel.x >= 0.f);

    // Gravity
    v.vel.y += gravity * dt;

    // Integrate position
    v.transform.localPosition.x += v.vel.x * dt;
    v.transform.localPosition.y += v.vel.y * dt;

    // Tile collision
    resolveVillainLevel(v, level);
    // animation state
    if(!v.grounded) {
        // airborne - no tick needed, jump is single frame
    } else if(std::abs(v.vel.x) > 0.1f) {
        setAnimation(v, v.walk);  // reusing player's setAnimation
        tickAnimation(v, v.walk);
    } else {
        setAnimation(v, v.idle);
        tickAnimation(v, v.idle);
    }

    // knife update
    Vec2 villainCenter = v.transform.localPosition + Vec2(VILLAIN_W / 2, VILLAIN_H / 2);
    Vec2 dir = playerPos - villainCenter;
    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    if(len > 0) {
        dir.x /= len;
        dir.y /= len;
    }
    v.knifeTransform.localAngle = atan2(dir.y, dir.x);
    v.knifeTransform.localPosition = Vec2(VILLAIN_W / 2, VILLAIN_H / 2) + dir * 25.0f;  // fixed offset distance
}


void drawVillain(Villain &v) {
    if(v.state == VillainState::DEAD) return;

    Vec2 drawSize(64, 64);
    if(!v.facingRight) drawSize.x = -64;

    Vec2 spriteOffset = v.facingRight ? Vec2(20, 16) : Vec2(-44, 16);

    if(!v.grounded) {
        drawTexture(v.jump, v.transform.localPosition - spriteOffset, drawSize);
    } else if(std::abs(v.vel.x) > 0.1f) {
        drawTexture(v.walk.frames[v.frame], v.transform.localPosition - spriteOffset, drawSize);
    } else {
        drawTexture(v.idle.frames[v.frame], v.transform.localPosition - spriteOffset, drawSize);
    }

    // Draw knife
    drawTexture(v.knifeTexture,
        v.transform.localPosition + v.knifeTransform.localPosition - Vec2(32, 32) / 2, Vec2(32, 32), v.knifeTransform.localAngle * 58 - 45);
}


bool hurtVillain(Villain &v) {
    if (v.state == VillainState::DEAD) return false;

    v.hp --;
    v.flashTimer = 0.15f;

    if (v.hp <= 0) {
        v.hp    = 0;
        v.state = VillainState::DEAD;
        return true;   
    }

    v.hurtTimer = 0.3f;
    v.state     = VillainState::HURT;
    // Knock back slightly
    v.vel.x = v.facingRight ? -120.f : 120.f;
    v.vel.y = -150.f;
    return false;
}


void resolveBulletsVillains(std::vector<Bullet> &bullets, std::vector<Villain> &villains) {
    for (Bullet &b : bullets) {
        if (!b.active) continue;
        for (Villain &v : villains) {
            if (v.state == VillainState::DEAD) continue;
            if (collision(b.transform.localPosition, b.size / 2.f,
                          v.transform.localPosition, v.size)) {
                b.active = false;
                hurtVillain(v);
                break;
            }
        }
    }
}
