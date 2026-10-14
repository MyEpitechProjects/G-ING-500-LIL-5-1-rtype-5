#include "objects/ShootLoadingObject.hpp"

rtype::ShootLoadingObject::ShootLoadingObject(uint16_t x, uint16_t y, uint16_t dir) : Object(x, y, dir)
{
}

void rtype::ShootLoadingObject::update(rtype::InputKeys& keys) {
    // todo
    if (keys.shoot) {
    } else {

    }
}