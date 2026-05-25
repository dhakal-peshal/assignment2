#ifndef player_h_
#define player_h_

#include <engine.h>
#include <transform.h>

const int PLAYER_SIZE_X = 20;
const int PLAYER_SIZE_Y = 48;

struct Animation {
    std::vector<Texture> frames;
    int no_frames;
    float duration;
    bool loop;
};
// using templates so functions can be used for both player and enemies
template<typename T>
void setAnimation(T &entity, Animation &anim) {
    if(entity.currentAnim != &anim) {
        entity.currentAnim = &anim;
        entity.animStart   = getTimeInSeconds();
        entity.frame       = 0;
    }
}
template<typename T>
void tickAnimation(T &entity, Animation &anim) {
    float elapsed = getTimeInSeconds() - entity.animStart;
    if(anim.loop) {
        elapsed = fmod(elapsed, anim.duration);
    } else {
        elapsed = std::min(elapsed, anim.duration);
    }
    entity.frame = (int)(elapsed / anim.duration * anim.no_frames);
    entity.frame = std::min(entity.frame, anim.no_frames - 1);
}

struct Player{
    Transform transform;
    Rect hitbox;
    Vec2 vel, size;
    Texture texture;
    int hp;
    bool grounded, onLeftWall, onRightWall;
    bool hasShotgun, hasBoot;
    int wallJumpDir;
    float wallJumpTimer = 0.0f;

    Animation idle, walk;
    Texture jump;
    float animStart;
    int frame;
    bool facingRight;
    Transform gunTransform;
    Texture gunTexture;
    Animation* currentAnim = nullptr;
};

void initPlayer(Player &player, Texture spritesheet);
//void setAnimation(Player &player, Animation &anim);
//void tickAnimation(Player &player, Animation &anim);
void recoil(Player &player, int amount);
void updatePlayer(Player &player, float dt);
void drawPlayer(Player &player);

#endif