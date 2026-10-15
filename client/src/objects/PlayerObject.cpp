#include "objects/PlayerObject.hpp"
#include "objects/NetworkObject.hpp"

rtype::PlayerObject::PlayerObject(uint16_t x, uint16_t y, uint16_t dir, rtype::ObjectManager& manager_obj) : Object(x, y, dir, manager_obj)
{
}

uint32_t rtype::PlayerObject::get_entity_id()
{
    return entity_id;
}

void rtype::PlayerObject::set_entity_id(uint32_t id)
{
    entity_id = id;
}

void rtype::PlayerObject::update(rtype::InputKeys& keys) {
    NetworkObject *obj = (NetworkObject *)manager.get_object("network");
    if (keys != old_keys){
        old_keys = keys;
        obj->send_input(keys);
    }
    if (keys.up && !keys.down) {
        set_animation("blue-up");
        if (is_moving_up == false){
            reverse_animation(false);
            reset_animation();
        }
        is_moving_up = true;
    }
    if (keys.down && !keys.up) {
        set_animation("blue-down");
        if (is_moving_down == false){
            reverse_animation(false);
            reset_animation();
        }
        is_moving_down = true;
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