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
Level level;
Texture spritesheet, playerSprites;
std::vector<Bullet> bullets;

// Initialise (called once at start)
void init() {
    setWindowTitle("Platformer");
    //SDL_ScaleModeNearest() = true;

    playerSprites = loadTexture("assets/player.png");
    spritesheet = loadTexture("assets/spritesheet.png");
    SDL_SetTextureScaleMode(playerSprites.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(spritesheet.texture, SDL_SCALEMODE_NEAREST);

    initPlayer(player, playerSprites);
    level = loadLevel({ // 1 = solid, 0 = empty, 40x22.5 tiles with current resolution
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000001111111111000000000000000000000000",
        "1000000000000000000001111100000000000000",
        "0000000000000000000000000000000111100000",
        "0000000000000000000000000000000000000000",
        "0001000000000000000000000000000000000000",
        "0000000000000000000000000011110000000000",
        "0000000000000000011111000000000000000000",
        "0000011110000000000000000000000000000000",
        "0000000000000000000000000000000000000000",
        "0000000000000111100000000000000000000000",
        "0000000000000000000000000000000000000000",
        "1111111111111111111111111111111111111111",
    });
}

void update(float dt) {
    updatePlayer(player, dt);
    resolvePlayerLevel(player, level);

    if(mouseButtonPressedThisFrame(MOUSE_BUTTON_LEFT)){
        createBullet(bullets, player.transform, spritesheet);
    }

    for(Bullet &bullet : bullets) {
        updateBullet(bullet, dt);
        resolveBulletLevel(bullet, level);
    }
}

void render(float lag) {
    clear(255,255,255); // background, change to texture in future
    drawLevel(level);
    //drawPlayer(player);
    drawPlayer(player);

    for(Bullet &bullet : bullets) {
        drawBullet(bullet);
    }
}

void close() {

}