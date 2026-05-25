#include <engine.h>
#include <iostream>
#include <vector>
#include <deque>

#include <transform.h>
#include <collision.h>

#include "player.h"
#include "level.h"
#include "bullet.h"
#include "villain.h"

Player player;
World world;
Texture spritesheet, playerSprites, enemySprites;
std::vector<Bullet> bullets;
std::vector<Villain> villains;

AudioClip pShot, sShot, sReload;
float sTimer, pTimer = 0.0f;
bool sReloading = false;

// Initialise (called once at start)
void init() {
    setWindowTitle("Platformer");

    // loading spritesheets and setting scale mode
    playerSprites = loadTexture("assets/player.png");
    enemySprites = loadTexture("assets/enemy.png");
    spritesheet = loadTexture("assets/spritesheet.png");
    SDL_SetTextureScaleMode(playerSprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(enemySprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(spritesheet.texture, SDL_SCALEMODE_NEAREST);

    initPlayer(player, playerSprites);
    world = loadWorld("assets/levels.json", spritesheet);
    spawnLevelEntities(currentLevel(world), villains, enemySprites);

    pShot   = loadAudioClip("./assets/audio/pistolshot.wav");
    sShot   = loadAudioClip("./assets/audio/shotgunshot.wav");
    sReload = loadAudioClip("./assets/audio/shotgun_reload.wav");
}

void update(float dt) {
    // player movement and collision
    updatePlayer(player, dt);
    resolvePlayerLevel(player, currentLevel(world));

    // fire bullet + bullet collision
    sTimer -= dt; pTimer -= dt;
    if (mouseButtonPressedThisFrame(MOUSE_BUTTON_LEFT) && pTimer <= 0.0f) {
        pTimer = 0.25f;
        createBullet(bullets, player.gunTransform, spritesheet, 1, 10.0f);
        recoil(player, 100);
        playOnce(pShot, 1.0f);
    }
    if (sReloading && sTimer <= 0.0f) {
        sReloading = false;
        playOnce(sReload, 1.0f);
    }
    if (mouseButtonPressedThisFrame(MOUSE_BUTTON_RIGHT) && sTimer <= 0.0f) {
        sTimer = 1.5f;
        createBullet(bullets, player.gunTransform, spritesheet, 5, 30.0f);
        recoil(player, 300);
        playOnce(sShot, 1.0f);
        sReloading = true;
    }
    for (Bullet &bullet : bullets) {
        updateBullet(bullet, dt);
        resolveBulletLevel(bullet, currentLevel(world));
    }

    // Villain update
    Vec2 playerCenter = player.transform.localPosition + Vec2(PLAYER_SIZE_X / 2, PLAYER_SIZE_Y / 2);
    for (Villain &v : villains) {
        updateVillain(v, playerCenter, player.hp, dt, currentLevel(world));
    }

    // Bullet <-> villain collision (25 dmg per bullet)
    resolveBulletsVillains(bullets, villains, 25);

    // world transition
    int next = checkLevelTransition(player, currentLevel(world));
    if (next != -1) {
        world.currentLevel = next;
        wrapPlayerPosition(player, currentLevel(world));
        villains.clear();
        bullets.clear();
        spawnLevelEntities(currentLevel(world), villains, enemySprites);
    }
}

void render(float lag) {
    clear(250,190,150); // background, change to texture in future
    //drawTexture(bg1, Vec2(0,0), Vec2(320, 180)*4);
    drawLevel(currentLevel(world));
    drawPlayer(player);

    for (Bullet &bullet : bullets) {
        drawBullet(bullet);
    }

    // Draw villains
    for (Villain &v : villains) {
        drawVillain(v);
    }
}

void close() {
}
