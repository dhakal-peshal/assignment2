#ifndef player_h_
#define player_h_

#include <engine.h>
#include <transform.h>

const int PLAYER_SIZE_X = 20;
const int PLAYER_SIZE_Y = 48;

struct Animation {
    std::vector<Texture> frames; // A vector (sequence) of individual frames
    int no_frames;               // Number of frames (just the length of the vector)
    float duration;              // Duration of animation
    bool loop;                   // Loop (does the animation loop or just play once)
};

struct Player{
    Transform transform;
    Rect hitbox;
    Vec2 vel, pos, size;
    Texture texture;
    int hp;
    bool grounded;

    Animation idle, walk;
    Texture jump;
    float animStart;
    int frame;
    bool facingRight;
};

void initPlayer(Player &player, Texture spritesheet);
void setAnimation(Player &player, Animation &anim);
void tickAnimation(Player &player, Animation &anim);
void updatePlayer(Player &player, float dt);
void drawPlayer(Player &player);

#endif