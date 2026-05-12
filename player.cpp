#include <algorithm>
#include "player.h"

void initPlayer(Player &player, Texture spritesheet) {
    player.transform.localPosition = Vec2(100, 100);
    player.vel = Vec2(0, 0);
    player.grounded = false;
    player.facingRight = true;
    player.frame = 0;
    player.animStart = getTimeInSeconds();

    // idle state
    for(int i = 0; i < 2; i++)
        player.idle.frames.push_back(subTexture(spritesheet, Rect{i * 16.0f, 0, 16, 16}));
    player.idle.no_frames = player.idle.frames.size();
    player.idle.duration = 0.8f;  // tweak this?
    player.idle.loop = true;

    // wakl state
    for(int i = 2; i < 6; i++)
        player.walk.frames.push_back(subTexture(spritesheet, Rect{i * 16.0f, 0, 16, 16}));
    player.walk.no_frames = player.walk.frames.size();
    player.walk.duration = 0.4f;
    player.walk.loop = true;

    // jump state
    player.jump = subTexture(spritesheet, Rect{6 * 16.0f, 0, 16, 16});
}

void setAnimation(Player &player, Animation &anim) {
    // only restart if switching to a different animation
    static Animation* current = nullptr;
    if(current != &anim) {
        current = &anim;
        player.animStart = getTimeInSeconds();
        player.frame = 0;
    }
}

void tickAnimation(Player &player, Animation &anim) {
    float elapsed = getTimeInSeconds() - player.animStart;
    if(anim.loop) {
        elapsed = fmod(elapsed, anim.duration);
    } else {
        elapsed = std::min(elapsed, anim.duration);
    }
    player.frame = (int)(elapsed / anim.duration * anim.no_frames);
    player.frame = std::min(player.frame, anim.no_frames - 1);  // guard against out-of-bounds
}

void updatePlayer(Player &player, float dt){
    float maxSpeed = 200.0f;
    float accel = 1200.0f;
    float friction = 800.0f;
    float gravity = 900.0f;
    float jump = -425.0f;
    // TODO: add coyoteTime, jumpBuffer, variableJumpHeight and ledgeHopping?

    // left, right and jump inputs
    float x_input = 0;
    if(keyIsPressed(KEY_D)) {x_input =  1.0f; }
    if(keyIsPressed(KEY_A)) {x_input =  -1.0f; }
    if(keyIsPressed(KEY_SPACE) && player.grounded) {
        player.vel.y = jump;
        //player.grounded = false;
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
    player.transform.localPosition.x += player.vel.x * dt;
    player.transform.localPosition.y += player.vel.y * dt;

    // flip direction
    if(player.vel.x > 0) player.facingRight = true;
    if(player.vel.x < 0) player.facingRight = false;

    // animation state
    if(!player.grounded) {
    } else if(std::abs(player.vel.x) > 0.1f) {
        setAnimation(player, player.walk);
        tickAnimation(player, player.walk);
    } else {
        setAnimation(player, player.idle);
        tickAnimation(player, player.idle);
    }
}

void drawPlayer(Player &player){
    drawRect(player.transform.localPosition, Vec2(PLAYER_SIZE_X, PLAYER_SIZE_Y), Color::red);
    Vec2 drawSize(64, 64);
    if (!player.facingRight) {drawSize.x = -64;}

    // draw state based on if grounded and velocity
    // due to sprite offset from origin, we need to move it basen on facing direction
    if(!player.grounded) {
        drawTexture(player.jump, player.facingRight ? 
            player.transform.localPosition - Vec2(20, 16) : player.transform.localPosition - Vec2(-44, 16), drawSize);

    } else if(std::abs(player.vel.x) > 0.1f) {
        drawTexture(player.walk.frames[player.frame], player.facingRight ? 
            player.transform.localPosition - Vec2(20, 16) : player.transform.localPosition - Vec2(-44, 16), drawSize);

    } else {
        drawTexture(player.idle.frames[player.frame], player.facingRight ? 
            player.transform.localPosition - Vec2(20, 16) : player.transform.localPosition - Vec2(-44, 16), drawSize);
    }
}