/*
** EPITECH PROJECT, 2026
** Registry.cpp
** File description:
** Registry class implementation
*/
#include "Registry.hpp"

void Registry::erase_entity(Entity const &entity) {
    for (const std::function<void(Registry &, Entity const &)> &eraser : _erasers)
        eraser(*this, entity);
}
