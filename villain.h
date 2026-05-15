#ifndef villain_h_
#define villain_h_

#include <engine.h>
#include <transform.h>
#include <vector>
#include "bullet.h"
#include "level.h"

const int VILLAIN_W = 24;
const int VILLAIN_H = 40;

// State machine for villain behaviour
enum class VillainState {
    PATROL,   // walking back and forth
    CHASE,    // running toward the player
    HURT,     // brief stun after taking a hit
    DEAD
};

struct Villain {
    Transform transform;
    Vec2 vel;
    Vec2 size;

    int hp;
    int maxHp;
    bool grounded;
    bool facingRight;

    VillainState state;

    float patrolDir;          // +1 or -1
    float patrolTimer;        // time until direction flip
    float hurtTimer;          // stun duration remaining
    float flashTimer;         // red flash duration remaining

    float chaseRange;         // pixel radius to start chasing
    float attackRange;        // pixel radius to damage player
    float attackCooldown;     // seconds between hits
    float attackTimer;
};

// Lifecycle
void initVillain(Villain &v, Vec2 startPos);
void updateVillain(Villain &v, Vec2 playerPos, int &playerHp, float dt, const LevelData &level);
void drawVillain(const Villain &v);

// Called when a bullet hits the villain; returns true if it killed it
bool hurtVillain(Villain &v, int damage);

// Check all bullets against all villains, deactivate bullets that hit
void resolveBulletsVillains(std::vector<Bullet> &bullets, std::vector<Villain> &villains, int bulletDamage = 25);

#endif
