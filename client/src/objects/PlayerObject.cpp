#include "objects/PlayerObject.hpp"

rtype::PlayerObject::PlayerObject(uint16_t x, uint16_t y, uint16_t dir) : Object(x, y, dir)
{
}

void rtype::PlayerObject::update(rtype::InputKeys& keys) {
    float deltaTime = objectClock.restart().asSeconds();
    if (keys.space){
        reset_animation();
    }
    if (keys.right) {
        move_x(speed * deltaTime);
    }
    if (keys.left) {
        move_x(-speed * deltaTime);
    }
    if (keys.up && !keys.down) {
        set_animation("blue-up");
        if (is_moving_up == false){
            reverse_animation(false);
            reset_animation();
        }
        is_moving_up = true;
        move_y(-speed * deltaTime);
    }
    if (keys.down && !keys.up) {
        set_animation("blue-down");
        if (is_moving_down == false){
            reverse_animation(false);
            reset_animation();
        }
        is_moving_down = true;
        move_y(speed * deltaTime);
    }
    if ((!keys.up && !keys.down) || (keys.up && keys.down)){
        if (is_moving_up || is_moving_down){
            reverse_animation(true);
            reset_animation();
        }
        is_moving_down = false;
        is_moving_up = false;
    }
}