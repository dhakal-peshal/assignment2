#ifndef collision_h_
#define collision_h_

#include <engine.h>

// Collision (Point-AABB)
bool collision(Vec2 pos0, Vec2 pos1, Vec2 size1);

// Collision (Circle-AABB)
bool collision(Vec2 pos0, float radius0, Vec2 pos1, Vec2 size1);

// Detect collision between a circle (pos0, radius0) and a rectangle with a rotation (pos1, size1, angle1)
bool collision(Vec2 pos0, float radius0, Vec2 pos1, Vec2 size1, float angle1);

// Collision (AABB-AABB)
bool collision(Vec2 pos0, Vec2 size0, Vec2 pos1, Vec2 size1);

// Collision (Rectangle-Rectangle)
bool collision(Vec2 pos0, Vec2 size0, float angle0, Vec2 pos1, Vec2 size1, float angle1);

#endif