#include <engine.h>
#include <iostream>
#include <vector>
#include <deque>

#include"player.h"
Player player;

//Texture spritesheet;
// Initialise (called once at start)
void init() {
    setWindowTitle("Platformer");
    initPlayer(player);
    //spritesheet = loadTexture("assets/spritesheet.png");
}

// Update Game
void update(float dt) {
    updatePlayer(player, dt);
}

// Render Game
void render(float lag) {
    // Background
    clear(0,0,0);
    drawPlayer(player);
}


// Close the Game
void close() {

}