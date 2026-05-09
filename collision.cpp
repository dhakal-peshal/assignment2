#include <collision.h>
#include <algorithm>

// Collision (Point-AABB)
bool collision(Vec2 pos0, Vec2 pos1, Vec2 size1) {
    return (pos0.x < pos1.x + size1.x) &&
            (pos1.x < pos0.x) &&
            (pos0.y < pos1.y + size1.y) &&
            (pos1.y < pos0.y);
}

// Collision (Circle-AABB)
bool collision(Vec2 pos0, float radius0, Vec2 pos1, Vec2 size1) {
    float tx, ty;

    // Check if circle is inside the rectangle
    if(collision(pos0, pos1, size1)) {
        return true;
    }

    // Find point on Rectangle closest to Circle (x)
    if(pos0.x < pos1.x) {
        tx = pos1.x; // Left Side
    } else if(pos0.x > pos1.x + size1.x) {
        tx = pos1.x + size1.x; // Right Side
    } else {
        tx = pos0.x; // Circle position
    }

    // Find point on Rectangle closest to Circle (y)
    if(pos0.y < pos1.y) {
        ty = pos1.y; // Top Side
    } else if(pos0.y > pos1.y + size1.y) {
        ty = pos1.y + size1.y; // Bottom Side
    } else {
        ty = pos0.y; // Circle position
    }

    // Compare distance between circle centre and closest point to radius
    if(distance(Vec2(tx, ty), pos0) < radius0) {
        return true;
    }
    return false;
}

// Detect collision between a circle (pos0, radius0) and a rectangle with a rotation (pos1, size1, angle1)
bool collision(Vec2 pos0, float radius0, Vec2 pos1, Vec2 size1, float angle1) {
    // Calculate the centre of the rectangle
    Vec2 centre = pos1 + size1/2;

    // Rotate circle 
    Vec2 pos0_rotated = centre + rotate(pos0 - centre, -angle1);
    
    // Use axis-aligned collision
    return collision(pos0_rotated, radius0, pos1, size1);
}

// Collision (AABB-AABB)
bool collision(Vec2 pos0, Vec2 size0, Vec2 pos1, Vec2 size1) {
    return (pos0.x < pos1.x + size1.x) &&
           (pos1.x < pos0.x + size0.x) &&
           (pos0.y < pos1.y + size1.y) &&
           (pos1.y < pos0.y + size0.y);
}

// Collision (AABB-Rectangle)
// (Not to be used directly)
bool collisionAABBRectangle(Vec2 pos0, Vec2 size0, Vec2 pos1, Vec2 size1, float angle1) {
    Vec2 centre1 = pos1 + size1/2;

    Vec2 a = centre1 + rotate(pos1 - centre1, angle1);
    Vec2 b = centre1 + rotate(pos1 + Vec2(size1.x, 0) - centre1, angle1);
    Vec2 c = centre1 + rotate(pos1 + Vec2(0, size1.y) - centre1, angle1);
    Vec2 d = centre1 + rotate(pos1 + size1 - centre1, angle1);

    float left = std::min({a.x,b.x, c.x,d.x});
    float right = std::max({a.x,b.x,c.x,d.x});
    float top = std::min({a.y,b.y,c.y,d.y});
    float bottom = std::max({a.y,b.y,c.y,d.y});

    return collision(pos0, size0, Vec2(left, top), Vec2(right-left, bottom-top));
}

// Collision (Rectangle-Rectangle)
bool collision(Vec2 pos0, Vec2 size0, float angle0, Vec2 pos1, Vec2 size1, float angle1) {
    // Centre of each rectangle
    Vec2 centre0 = pos0 + size0/2;
    Vec2 centre1 = pos1 + size1/2;

    // Rotate each rectangle to be relative to the other.
    Vec2 pos0_r = centre1 + rotate(centre0 - centre1, -angle1) - size0/2;
    Vec2 pos1_r = centre0 + rotate(centre1 - centre0, -angle0) - size1/2;

    // Check that both AABBs detect a collision
    return collisionAABBRectangle(pos0, size0, pos1_r, size1, angle1-angle0) &&
           collisionAABBRectangle(pos1, size1, pos0_r, size0, angle0-angle1);
}