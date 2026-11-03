/*
** EPITECH PROJECT, 2026
** EnemySpawnSystem
** File description:
** EnemySpawnSystem
*/

#include "GameSystems.hpp"
#include "Enemy.hpp"
#include "EntityHitbox.hpp"
#include "Health.hpp"
#include "Position.hpp"
#include "Velocity.hpp"

void rtype::EnemySpawnSystem::operator()(Registry &registry, Entity enemy, int x, int y) const {
    registry.addComponent(enemy, Position{x, y});
    registry.addComponent(enemy, Velocity{-2, 0});
    registry.addComponent(enemy, ENTITY_HITBOX);
    registry.addComponent(enemy, Health{1});
    registry.addComponent(enemy, Enemy{});
}
