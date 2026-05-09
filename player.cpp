#include <algorithm>
#include "player.h"

void initPlayer(Player &player){
    player.pos.x = 100;
    player.pos.y = 100;
    player.vel = Vec2(0,0);
    player.grounded = false;
}

void updatePlayer(Player &player, float dt){
    float maxSpeed = 200.0f;
    float accel = 1200.0f;
    float friction = 800.0f;
    float gravity = 900.0f;
    float jump = -425.0f;
    // TODO: add coyoteTime, jumpBuffer, variableJumpHeight and ledgeHopping?

    // left/right and jump inputs
    float x_input = 0;
    if(keyIsPressed(KEY_D)) {x_input =  1.0f; }
    if(keyIsPressed(KEY_A)) {x_input =  -1.0f; }
    if(keyIsPressed(KEY_SPACE) && player.grounded) {
        player.vel.y = jump;
        player.grounded = false;
    }
    // acceleration
    if(x_input != 0) {
        player.vel.x += x_input * accel * dt;
        if(player.vel.x >  maxSpeed) player.vel.x =  maxSpeed;
        else if(player.vel.x < -maxSpeed) player.vel.x = -maxSpeed;
    } else {
        // friction when no input
        float frictionStep = friction * dt;
        if(std::abs(player.vel.x) <= frictionStep)
            player.vel.x = 0;
        else
            player.vel.x -= std::copysign(frictionStep, player.vel.x);
    }

    // gravity and player movement
    player.vel.y += gravity * dt;
    player.pos.x += player.vel.x * dt;
    player.pos.y += player.vel.y * dt;
}

void drawPlayer(Player &player){
    drawRect(player.pos, Vec2(PLAYER_SIZE, PLAYER_SIZE), Color::red);
}