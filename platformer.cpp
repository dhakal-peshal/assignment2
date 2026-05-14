#include <engine.h>
#include <iostream>
#include <vector>
#include <deque>

#include <transform.h>
#include <collision.h>

#include"player.h"
#include"level.h"
#include"bullet.h"

Player player;
World world;
Texture spritesheet, playerSprites;
std::vector<Bullet> bullets;

// Initialise (called once at start)
void init() {
    setWindowTitle("Platformer");

    playerSprites = loadTexture("assets/player.png");
    spritesheet = loadTexture("assets/spritesheet.png");
    SDL_SetTextureScaleMode(playerSprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(spritesheet.texture, SDL_SCALEMODE_NEAREST);

    initPlayer(player, playerSprites);
    world = loadWorld("assets/levels.json", spritesheet);
}

void update(float dt) {
    // player movement and collision
    updatePlayer(player, dt);
    resolvePlayerLevel(player, currentLevel(world));

    // fire bullet + bullet collision
    if(mouseButtonPressedThisFrame(MOUSE_BUTTON_LEFT)){
        createBullet(bullets, player.gunTransform, spritesheet);
    }
    for(Bullet &bullet : bullets) {
        updateBullet(bullet, dt);
        resolveBulletLevel(bullet, currentLevel(world));
    }

    // world transition
    int next = checkLevelTransition(player, currentLevel(world));
    if(next != -1) {
        world.currentLevel = next;
        wrapPlayerPosition(player, currentLevel(world));  // see below
    }
}

void render(float lag) {
    clear(255,255,255); // background, change to texture in future
    drawLevel(currentLevel(world));
    //drawPlayer(player);
    drawPlayer(player);

    for(Bullet &bullet : bullets) {
        drawBullet(bullet);
    }
}

void close() {

}