#include "objects/ShootLoadingObject.hpp"

rtype::ShootLoadingObject::ShootLoadingObject(uint16_t x, uint16_t y, uint16_t dir, rtype::ObjectManager& manager_obj) : Object(x, y, dir, manager_obj)
{
}

void rtype::ShootLoadingObject::update(rtype::InputKeys& keys) {
    if (keys.shoot) {
        rtype::Object* player = manager.get_object("player");
        set_x(player->get_x() + 30);
        set_y(player->get_y() - 5);
        if (!is_pressed){
            reset_animation();
            is_pressed = true;
            objectClock.restart();
        } else if (objectClock.getElapsedTime().asMilliseconds() > 250) {
            show();
        } else {
            hide();
        }
        is_pressed = true;
    } else {
        is_pressed = false;
        hide();
    }
}