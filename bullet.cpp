#include <bullet.h>
#include "player.h"

// create a bullet
void createBullet(std::vector<Bullet> &bullets, Transform transform, Texture spritesheet, int shots, float spread = 0.0f) {
    Bullet bullet;
    bullet.transform = transform;
    bullet.transform.parent = nullptr;
    
    for(int i; i < shots; i++) { // if multiple shots needed
        Vec2 dir = mousePosition() - transform.position();
        float len = sqrt(dir.x * dir.x + dir.y * dir.y);
        if(len > 0) dir = dir / len;

        // add spread
        if(spread > 0.0f) {
            float randomAngle = ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * spread;
            dir = rotate(dir, randomAngle);
        }

        bullet.vel = dir * 1000.0f;
        bullet.transform.localPosition = transform.position() + dir * 8;
        bullet.texture = subTexture(spritesheet, {0, 16, 8, 8});
        bullet.size = Vec2(8, 8);
        bullet.active = true;
        
        bullets.push_back(bullet);
    }
    
}

// update bullet
void updateBullet(Bullet &bullet, float dt) {
    //mousePosition
    if(bullet.active) {
        // Move bullet
        bullet.transform.localPosition += bullet.vel * dt;

        // Destroy at Boundary
        if(bullet.transform.position().x < 0) bullet.active = false;
        if(bullet.transform.position().x > WINDOW_WIDTH) bullet.active = false;
        if(bullet.transform.position().y < 0) bullet.active = false;
        if(bullet.transform.position().y > WINDOW_HEIGHT) bullet.active = false;
    }
}

// draw bullet
void drawBullet(Bullet &bullet) {
    // If bullet is active
    if(bullet.active) {
        // Draw Texture and Hitbox
        drawTexture(bullet.texture, bullet.transform.position() - Vec2(20,12), Vec2(2,2) * 16, bullet.transform.angle());
        //drawRect(bullet.transform.position() - bullet.size/2, bullet.size, Color::red, bullet.transform.angle());
    }
}