#include <transform.h>

Transform::Transform() {
    parent = nullptr;
    localPosition = Vec2(0,0);
    localAngle = 0.0f;
    localScale = Vec2(1,1);
}

Transform::Transform(Transform *par, Vec2 pos, float angle, Vec2 scale) {
    parent = par;
    if(parent != nullptr) {
        parent->addChild(this);
    }

    localPosition = pos;
    localAngle = angle;
    localScale = scale;
}

void Transform::addChild(Transform *child) {
    children.push_back(child);
    child->parent = this;
}

Vec2 Transform::position(SPACE space) {
    if((space == LOCAL) || (parent == nullptr)) {
        // Local position
        return localPosition;
    } else {
        // Global position
        return parent->position(GLOBAL) + localPosition.x * parent->right(GLOBAL) * parent->scale(GLOBAL).x
                                        + localPosition.y * parent->up(GLOBAL) * parent->scale(GLOBAL).y;
    }
}

Vec2 Transform::up(SPACE space) {
    if((space == LOCAL) || (parent == nullptr)) {
        // Local up vector
        return ::rotate(Vec2(0,1), localAngle);
    } else {
        // Global up vector
        return ::rotate(Vec2(0,1), localAngle + parent->angle(GLOBAL));
    }
}

Vec2 Transform::right(SPACE space) {
    if((space == LOCAL) || (parent == nullptr)) {
        // Local right vector
        return ::rotate(Vec2::right, localAngle);
    } else {
        // Global right vector
        return ::rotate(Vec2::right, localAngle + parent->angle(GLOBAL));
    }
}

float Transform::angle(SPACE space) {
    if((space == LOCAL) || (parent == nullptr)) {
        // Local angle
        return localAngle;
    } else {
        // Global angle
        return localAngle + parent->angle(GLOBAL);
    }
}

Vec2 Transform::scale(SPACE space) {
    if((space == LOCAL) || (parent == nullptr)) {
        // Local scale
        return localScale;
    } else {
        // Global angle
        Vec2 parentScale = parent->scale(GLOBAL);
        return Vec2(localScale.x * parentScale.x, localScale.y * parentScale.y);
    }
}


void Transform::translate(Vec2 t, SPACE space) {
    if(space == LOCAL) {
        if(parent == nullptr) {
            // Translate based on local rotation
            localPosition += t.x * right(GLOBAL) + t.y * up(GLOBAL);
        } else {
            // Translate based on local rotation
            localPosition += t.x * right(LOCAL) + t.y * up(LOCAL);
        }
    } else {
        // Translate in global space
        if(parent == nullptr) {
            // Move position
            localPosition += t;
        } else {
            // Get parent angle in global space
            float globalAngle = parent->angle(GLOBAL);
            
            // Apply the inverse rotation and add to position
            localPosition += ::rotate(t, -globalAngle);
        }
    }
}

void Transform::rotate(float angle, SPACE space) {
    // Space doesn't actually make any difference here.
    // There is only one axis of rotation
    localAngle += angle;
}

void Transform::scale(Vec2 scale, SPACE space) {
    // Local scale
    localScale = Vec2(localScale.x * scale.x, localScale.y * scale.y);
}