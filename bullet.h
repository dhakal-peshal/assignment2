#ifndef bullet_h_
#define bullet_h_

#include <engine.h>
#include <transform.h>

// Bullet Structure
struct Bullet {
    Transform transform;
    Vec2 vel, size;
    bool active;
    Texture texture;
};

// Create a bullet
void createBullet(std::vector<Bullet> &bullets, Transform transform, Texture spritesheet);

// Update bullet
void updateBullet(Bullet &bullet, float dt);

// Draw bullet
void drawBullet(Bullet &bullet);

#endif