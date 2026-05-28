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

enum GameState {
    STATE_MAIN_MENU,
    STATE_PLAYING
};

GameState gameState = STATE_MAIN_MENU;

Texture menuBackground;
AudioClip menuClick;

struct Button {
    float x, y, w, h;
    const char *label;
};
// As for game name we can just make a pnng and stick it in the background, Less code to do XD
Button btnStart = { 490, 310, 300, 70, "START" };
Button btnQuit  = { 490, 410, 300, 70, "QUIT"  };

bool isHovered(const Button &b) {
    Vec2 m = mousePosition();
    return m.x >= b.x && m.x <= b.x + b.w &&
           m.y >= b.y && m.y <= b.y + b.h;
}

void drawButton(const Button &b, bool hovered) {
    fillRect(b.x + 4, b.y + 4, b.w, b.h, 0, 0, 0, 120);
    if (hovered)
        fillRect(b.x, b.y, b.w, b.h, 220, 80,  20, 230);
    else
        fillRect(b.x, b.y, b.w, b.h, 30,  10,  10, 200);
    drawRect(b.x, b.y, b.w, b.h, 255, 160, 60, 255);
    int labelX = (int)(b.x + b.w / 2 - (strlen(b.label) * 4));
    int labelY = (int)(b.y + b.h / 2 - 4);
    drawText((float)labelX, (float)labelY, (char*)b.label, 255, 230, 160, 255);
}

Player player;
World world;
Texture spritesheet, playerSprites, enemySprites, shotgun, boot, item_sg, item_b, item_h, deathScreen;
std::vector<Bullet> bullets;
std::vector<Villain> villains;

AudioClip pShot, sShot, sReload;
float sTimer, pTimer = 0.0f;
bool sReloading = false;

// pickup text variables
bool showTextBox = false;
char textBoxPickup = ' ';
float textBoxTimer = 0.0f;
const float TEXT_BOX_DURATION = 5.0f;

// Initialise (called once at start)
void initGame() {
    

    // loading spritesheets and setting scale mode
    playerSprites = loadTexture("assets/player.png");
    enemySprites = loadTexture("assets/enemy.png");
    spritesheet = loadTexture("assets/spritesheet.png");
    shotgun = loadTexture("assets/shotgun.png");
    boot = loadTexture("assets/boot.png");
    deathScreen = loadTexture("assets/death.png");

    SDL_SetTextureScaleMode(playerSprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(enemySprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(spritesheet.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(shotgun.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(boot.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(deathScreen.texture, SDL_SCALEMODE_NEAREST);
    // item textures
    item_sg = subTexture(spritesheet, {8, 16, 8, 8});
    item_b = subTexture(spritesheet, {16, 16, 8, 8});
    item_h = subTexture(spritesheet, {24, 16, 8, 8});

    initPlayer(player, playerSprites);
    world = loadWorld("assets/levels.json", spritesheet);
    spawnLevelEntities(currentLevel(world), villains, enemySprites);

    pShot = loadAudioClip("./assets/audio/pistolshot.wav");
    sShot = loadAudioClip("./assets/audio/shotgunshot.wav");
    sReload = loadAudioClip("./assets/audio/shotgun_reload.wav");
}

void respawnPlayer(Player &player, World &world, std::vector<Villain> &villains, std::vector<Bullet> &bullets, Texture enemySprites) {
    player.dead = false;
    player.respawnTimer = 0.0f;
    player.hp = 5;

    // Return to start room and reset position to start coordinates
    if (!player.hasShotgun && !player.hasBoot) {
        world.currentLevel = 0;
    player.transform.localPosition = Vec2(100, 550);
    } else if (player.hasShotgun && !player.hasBoot) {
        world.currentLevel = 3;
        player.transform.localPosition = Vec2(50, 200);
    } else if (player.hasShotgun && player.hasBoot) {
        world.currentLevel = 8;
    player.transform.localPosition = Vec2(50, 50);
    }
    
    villains.clear();
    spawnLevelEntities(currentLevel(world), villains, enemySprites);
    bullets.clear();
    player.vel = Vec2(0, 0);
}
void init() {
    setWindowTitle ("Platformer");
    menuBackground = loadTexture("assets/background.png");
    menuClick = loadAudioClip("./assets/audio/menu_click.wav");
}
void update(float dt) {
    if (gameState == STATE_MAIN_MENU) {
        if (mouseButtonPressedThisFrame(MOUSE_BUTTON_LEFT)) {
            if (isHovered(btnStart)) {
                playOnce(menuClick, 1.0f);
                initGame();
                gameState = STATE_PLAYING;
            } else if (isHovered(btnQuit)) {
                playOnce(menuClick, 1.0f);
                SDL_Quit();
                exit(0);
            }
        }
        return;
    }

    if(player.dead) {
        updatePlayer(player, dt);  // ticks respawnTimer
        if(player.respawnTimer <= 0.0f)
            respawnPlayer(player, world, villains, bullets, enemySprites);
        return;  // skip rest of update while dead
    }
    // player movement and collision
    updatePlayer(player, dt);
    resolvePlayerLevel(player, currentLevel(world));
    resolvePickups(currentLevel(world), player, showTextBox, textBoxPickup, textBoxTimer, TEXT_BOX_DURATION);

    if(showTextBox) {
        textBoxTimer -= dt;
        if(textBoxTimer <= 0.0f) showTextBox = false;
    }

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
    if (mouseButtonPressedThisFrame(MOUSE_BUTTON_RIGHT) && sTimer <= 0.0f && player.hasShotgun) {
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

    // bullet to villain collision
    resolveBulletsVillains(bullets, villains);

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
    if (gameState == STATE_MAIN_MENU) {
        drawTexture(menuBackground, 0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT);
        drawButton(btnStart, isHovered(btnStart));
        drawButton(btnQuit,  isHovered(btnQuit));
        return;
    }

    clear(250,190,150); // background, change to texture in future
    drawLevel(currentLevel(world));
    drawPlayer(player);

    for (Bullet &bullet : bullets) {
        drawBullet(bullet);
    }

    // draw villains
    for (Villain &v : villains) {
        drawVillain(v);
    }

    // draw pickups
    for(PickupData &pickup : currentLevel(world).pickups) {
        if(!pickup.active) continue;
        Vec2 pos(pickup.col * TILE_SIZE, pickup.row * TILE_SIZE);
        if (pickup.type == 's') drawTexture(item_sg, pos, Vec2(TILE_SIZE, TILE_SIZE));
        else if (pickup.type == 'b') drawTexture(item_b, pos, Vec2(TILE_SIZE, TILE_SIZE));
        else drawTexture(item_h, pos, Vec2(TILE_SIZE, TILE_SIZE));
    }

    // draw text box
    if(showTextBox) {
        if(textBoxPickup == 's')
            drawTexture(shotgun, Vec2(440, 240), Vec2(400, 240));
        else // only 2 items exist so no more specific state rquired
            drawTexture(boot, Vec2(440, 240), Vec2(400, 240));
    }

    if(player.dead) {
        drawTexture(deathScreen, Vec2(400, 200), Vec2(480, 320));
    }
}

void close() {}
