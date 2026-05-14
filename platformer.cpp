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
AudioClip pShot, sShot, sReload;
float sTimer, pTimer = 0.0f;
bool sReloading = false;

// Initialise (called once at start)
void init() {
    setWindowTitle("Platformer");

    playerSprites = loadTexture("assets/player.png");
    spritesheet = loadTexture("assets/spritesheet.png");
    SDL_SetTextureScaleMode(playerSprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(spritesheet.texture, SDL_SCALEMODE_NEAREST);

    initPlayer(player, playerSprites);
    world = loadWorld("assets/levels.json", spritesheet);

    pShot = loadAudioClip("./assets/audio/pistolshot.wav");
    sShot = loadAudioClip("./assets/audio/shotgunshot.wav");
    sReload = loadAudioClip("./assets/audio/shotgun_reload.wav");
}

void update(float dt) {
    // player movement and collision
    updatePlayer(player, dt);
    resolvePlayerLevel(player, currentLevel(world));

    // fire bullet + bullet collision
    //while (reloadTimer > 0) {reloadTimer -= dt;}
    sTimer -= dt; pTimer -= dt;
    if(mouseButtonPressedThisFrame(MOUSE_BUTTON_LEFT) && pTimer <= 0.0f){
        pTimer = 0.25f;
        createBullet(bullets, player.gunTransform, spritesheet, 1, 10.0f);
        recoil(player, 100);
        playOnce(pShot, 1.0f);
    }
    if (sReloading && sTimer <= 0.0f) {
    sReloading = false;
    playOnce(sReload, 1.0f);
    }
    if(mouseButtonPressedThisFrame(MOUSE_BUTTON_RIGHT) && sTimer <= 0.0f){
        sTimer = 1.5f;
        createBullet(bullets, player.gunTransform, spritesheet, 5, 30.0f);
        recoil(player, 300);
        playOnce(sShot, 1.0f);
        sReloading = true;
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
    clear(250,190,150); // background, change to texture in future
    drawLevel(currentLevel(world));
    //drawPlayer(player);
    drawPlayer(player);

    for(Bullet &bullet : bullets) {
        drawBullet(bullet);
    }
}

void close() {

}