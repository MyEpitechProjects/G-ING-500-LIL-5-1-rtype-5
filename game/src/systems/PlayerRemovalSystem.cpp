/*
** EPITECH PROJECT, 2026
** PlayerRemovalSystem
** File description:
** PlayerRemovalSystem
*/

#include "GameSystems.hpp"
#include "Player.hpp"

void rtype::PlayerRemovalSystem::operator()(Registry &registry, Entity player, const std::function<void(Entity)> &destroyEntity) const {
    if (registry.hasComponent<Player>(player))
        destroyEntity(player);
}
