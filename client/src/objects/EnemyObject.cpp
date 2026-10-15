#include "objects/EnemyObject.hpp"

rtype::EnemyObject::EnemyObject(uint16_t x, uint16_t y, uint16_t dir, rtype::ObjectManager& manager_obj) : Object(x, y, dir, manager_obj)
{
}

uint32_t rtype::EnemyObject::get_entity_id()
{
    return entity_id;
}

void rtype::EnemyObject::set_entity_id(uint32_t id)
{
    entity_id = id;
}

void rtype::EnemyObject::update(rtype::InputKeys& keys)
{
    (void)keys;
}