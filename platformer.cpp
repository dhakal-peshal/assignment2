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
Texture sheriffTexture;
Texture dialogueBoxTexture;
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
struct TutorialLine {
    const char *text;
};

const TutorialLine TUTORIAL_STEPS[] = {
    { "Howdy partner! Welcome to the Ranch!" },
    { "Use A amd D to move left & right, and Space to jump!" },
    { "Left-click your mouse to fire your trusty pistol." },
    { "Also find the Wall walking boots!" },
    { "Pro tip: shoot while jumping for an inertia boost!" },
    { "Now get out there, cowboy. Good luck!" },
};
const int TUTORIAL_STEP_COUNT = 6;

int  tutorialStep    = 0;      
bool tutorialActive  = true;   
float tutorialSlide  = 0.0f;   
bool tutorialClosing = false;  

Player player;
World world;
Texture spritesheet, playerSprites, enemySprites, shotgun, boot, item_sg, item_b, item_h, deathScreen3, deathScreen2, deathScreen1;
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
    deathScreen3 = loadTexture("assets/death.png");
    deathScreen2 = loadTexture("assets/death 2.png");
    deathScreen1 = loadTexture("assets/death 1.png");

    SDL_SetTextureScaleMode(playerSprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(enemySprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(spritesheet.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(shotgun.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(boot.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(deathScreen3.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(deathScreen2.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(deathScreen1.texture, SDL_SCALEMODE_NEAREST);
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

    tutorialStep    = 0;
    tutorialActive  = true;
    tutorialSlide   = 0.0f;
    tutorialClosing = false;
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
void drawTutorial(float slide) {
    if (slide <= 0.0f) return;

    const float panelW   = 560.0f;
    const float panelH   = 160.0f;
    const float sheriffW = 110.0f;
    const float sheriffH = 150.0f;   
    const float margin   = 18.0f;

    
    float panelX = margin;
    float panelY = (float)WINDOW_HEIGHT - margin - panelH;
    float offsetY = (1.0f - slide) * (panelH + margin);
    panelY += offsetY;

    
    fillRect(panelX + 5, panelY + 5, panelW, panelH, 0, 0, 0, 100);

    
    drawTexture(dialogueBoxTexture, Vec2(panelX, panelY), Vec2(panelW, panelH));

    
    float sx = panelX + 8.0f;
    float sy = panelY + (panelH - sheriffH) / 2.0f;
    drawTexture(sheriffTexture, Vec2(sx, sy), Vec2(sheriffW, sheriffH));

    
    float textX    = panelX + sheriffW + 18.0f;
    float textAreaW = panelW - sheriffW - 30.0f;

    
    drawText(textX, panelY + 12.0f, (char*)"Sheriff Sam", 0, 0, 0, 255);

    
    fillRect(textX, panelY + 28.0f, textAreaW, 1.5f, 0, 0, 0, 255);

    
    const char *line = TUTORIAL_STEPS[tutorialStep].text;
    drawText(textX, panelY + 38.0f, (char*)line, 0, 0, 0, 255);

    
    char stepBuf[16];
    snprintf(stepBuf, sizeof(stepBuf), "%d / %d", tutorialStep + 1, TUTORIAL_STEP_COUNT);
    drawText(panelX + panelW - 48.0f, panelY + panelH - 30.0f, stepBuf, 0, 0, 0, 255);

    
    const char *prompt = (tutorialStep < TUTORIAL_STEP_COUNT - 1)
                         ? "[ Click to continue ]"
                         : "[ Click to close ]";
    drawText(textX, panelY + panelH - 32.0f, (char*)prompt, 0, 0, 0, 255);
}

void init() {
    setWindowTitle ("Platformer");
    menuBackground = loadTexture("assets/background.png");
    sheriffTexture     = loadTexture("assets/sheriff.png");
    dialogueBoxTexture = loadTexture("assets/dialougebox.png");
    SDL_SetTextureScaleMode(sheriffTexture.texture,     SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(sheriffTexture.texture,     SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(dialogueBoxTexture.texture, SDL_SCALEMODE_NEAREST);
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
//for sliding effect
    if (tutorialActive) {
        if (!tutorialClosing)
            tutorialSlide = tutorialSlide + dt * 4.0f;   // slide in
        else
            tutorialSlide = tutorialSlide - dt * 4.0f;   // slide out

        if (tutorialSlide > 1.0f) tutorialSlide = 1.0f;

        if (tutorialClosing && tutorialSlide <= 0.0f) {
            tutorialActive  = false;
            tutorialClosing = false;
            tutorialSlide   = 0.0f;
        }

        //  left-click close
        if (mouseButtonPressedThisFrame(MOUSE_BUTTON_LEFT) && !tutorialClosing) {
            playOnce(menuClick, 0.6f);
            tutorialStep++;
            if (tutorialStep >= TUTORIAL_STEP_COUNT) {
                tutorialStep    = TUTORIAL_STEP_COUNT - 1;
                tutorialClosing = true;
            }
        }
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
         { Texture &ds = (player.respawnTimer > 2.0f) ? deathScreen3 : (player.respawnTimer > 1.0f) ? deathScreen2 : deathScreen1; drawTexture(ds, Vec2(400, 200), Vec2(480, 320)); }
    }
    if (tutorialActive)
        drawTutorial(tutorialSlide);
}

void close() {}
