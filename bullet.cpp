#include <bullet.h>

// Create a bullet
void createBullet(std::vector<Bullet> &bullets, Transform transform, Texture spritesheet) {
    Bullet bullet;
    bullet.transform = transform;
    
    Vec2 dir = mousePosition() - transform.localPosition;
    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    if(len > 0) dir = dir / len;

    bullet.vel = dir * 1000.0f;
    bullet.transform.localPosition += dir * 20;
    bullet.texture = subTexture(spritesheet, {8, 32, 8, 8});
    bullet.size = Vec2(8, 8);
    bullet.active = true;
    
    bullets.push_back(bullet);
}

// Update bullet
void updateBullet(Bullet &bullet, float dt) {
    // If bullet Is Active
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

// Draw bullet
void drawBullet(Bullet &bullet) {
    // If bullet is active
    if(bullet.active) {
        // Draw Texture and Hitbox
        drawTexture(bullet.texture, bullet.transform.position() - Vec2(20,12), Vec2(2,2) * 16, bullet.transform.angle());
        //drawRect(bullet.transform.position() - bullet.size/2, bullet.size, Color::red, bullet.transform.angle());
    }
}