#include "villain.h"
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

bool edgeAhead(const Villain &v, const LevelData &level) {
    float footX = v.transform.localPosition.x + (v.facingRight ? v.size.x + 2.f : -2.f);
    float footY = v.transform.localPosition.y + v.size.y + 4.f;

    int col = (int)(footX / TILE_SIZE);
    int row = (int)(footY / TILE_SIZE);

    if (col < 0 || col >= level.cols || row < 0 || row >= level.rows) return true;
    return !tileSolid(level, col, row);
}


void initVillain(Villain &v, Vec2 startPos) {
    v.transform.localPosition = startPos;
    v.transform.localAngle    = 0.f;
    v.transform.localScale    = Vec2(1, 1);
    v.transform.parent        = nullptr;

    v.vel        = Vec2(0, 0);
    v.size       = Vec2(VILLAIN_W, VILLAIN_H);
    v.hp         = 100;
    v.maxHp      = 100;
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
            playerHp     -= 10;
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
}


void drawVillain(const Villain &v) {
    if (v.state == VillainState::DEAD) return;

    Vec2 pos  = v.transform.localPosition;
    bool flash = (v.flashTimer > 0.f);

    Color bodyColor = flash ? Color(220, 40, 40) : Color(80, 30, 120);

    fillRect(pos, v.size, bodyColor);

    float eyeY  = pos.y + 10.f;
    float eyeRX = v.facingRight ? pos.x + 16.f : pos.x + 6.f;
    float eyeLX = v.facingRight ? pos.x + 10.f : pos.x;
    fillCircle(eyeRX, eyeY, 4.f, Color(255, 60, 60));
    fillCircle(eyeLX, eyeY, 4.f, Color(255, 60, 60));

    float mouthY = pos.y + 22.f;
    if (v.facingRight) {
        drawLine(Vec2(pos.x + 6.f, mouthY + 3.f), Vec2(pos.x + 12.f, mouthY), Color(255, 180, 180));
        drawLine(Vec2(pos.x + 12.f, mouthY), Vec2(pos.x + 18.f, mouthY + 3.f), Color(255, 180, 180));
    } else {
        drawLine(Vec2(pos.x + 4.f, mouthY + 3.f), Vec2(pos.x + 10.f, mouthY), Color(255, 180, 180));
        drawLine(Vec2(pos.x + 10.f, mouthY), Vec2(pos.x + 16.f, mouthY + 3.f), Color(255, 180, 180));
    }

    drawLine(Vec2(pos.x + 4.f,  pos.y),       Vec2(pos.x + 2.f, pos.y - 10.f), Color(200, 80, 0));
    drawLine(Vec2(pos.x + 18.f, pos.y),       Vec2(pos.x + 20.f, pos.y - 10.f), Color(200, 80, 0));

    float legAnim = (v.state == VillainState::PATROL || v.state == VillainState::CHASE)
                    ? std::sin(getTimeInSeconds() * 8.f) * 5.f : 0.f;
    float legY = pos.y + v.size.y;
    fillRect(Vec2(pos.x + 2.f,  legY), Vec2(8, 6 + legAnim),  Color(60, 20, 90));
    fillRect(Vec2(pos.x + 14.f, legY), Vec2(8, 6 - legAnim),  Color(60, 20, 90));

    float barW   = 30.f;
    float barH   = 4.f;
    float barX   = pos.x + (v.size.x - barW) / 2.f;
    float barY   = pos.y - 12.f;
    float filled = barW * ((float)v.hp / (float)v.maxHp);

    fillRect(Vec2(barX, barY), Vec2(barW, barH), Color(60, 0, 0));
    if (filled > 0.f)
        fillRect(Vec2(barX, barY), Vec2(filled, barH), Color(220, 50, 50));
    drawRect(Vec2(barX, barY), Vec2(barW, barH), Color(200, 200, 200));
}


bool hurtVillain(Villain &v, int damage) {
    if (v.state == VillainState::DEAD) return false;

    v.hp -= damage;
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


void resolveBulletsVillains(std::vector<Bullet> &bullets, std::vector<Villain> &villains, int bulletDamage) {
    for (Bullet &b : bullets) {
        if (!b.active) continue;
        for (Villain &v : villains) {
            if (v.state == VillainState::DEAD) continue;
            if (collision(b.transform.localPosition, b.size / 2.f,
                          v.transform.localPosition, v.size)) {
                b.active = false;
                hurtVillain(v, bulletDamage);
                break;
            }
        }
    }
}
