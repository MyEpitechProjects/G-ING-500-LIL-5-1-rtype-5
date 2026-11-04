/*
** EPITECH PROJECT, 2026
** Hitbox
** File description:
** Hitbox def
*/

#ifndef HITBOX
    #define HITBOX

    #include "Position.hpp"
    #include <cstdlib>

struct Hitbox {
    int halfWidth;
    int halfHeight;
};

inline bool overlaps(const Position &leftPosition, const Hitbox &leftHitbox, const Position &rightPosition, const Hitbox &rightHitbox) {
    return std::abs(leftPosition.x - rightPosition.x) <= leftHitbox.halfWidth + rightHitbox.halfWidth
        && std::abs(leftPosition.y - rightPosition.y) <= leftHitbox.halfHeight + rightHitbox.halfHeight;
}

#endif
