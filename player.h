#ifndef player_h_
#define player_h_

#include <engine.h>
#include <transform.h>

const int PLAYER_SIZE = 32;

struct Player{
    Transform transform;
    Rect hitbox;
    Vec2 vel;
    Vec2 pos;
    Vec2 size;
    Texture texture;
    int hp;
    bool grounded;
};

void initPlayer(Player &player);
void updatePlayer(Player &player, float dt);
void drawPlayer(Player &player);

#endif