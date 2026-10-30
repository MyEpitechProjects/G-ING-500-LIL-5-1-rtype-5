/*
** EPITECH PROJECT, 2026
** ProjectileSpawnSystem
** File description:
** ProjectileSpawnSystem
*/

#include "GameSystems.hpp"
#include "EntityHitbox.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Velocity.hpp"

void rtype::ProjectileSpawnSystem::operator()(Registry &registry, Entity projectile, int x, int y, bool fromPlayer) const {
    registry.addComponent(projectile, Position{x, y});
    registry.addComponent(projectile, Velocity{fromPlayer ? 13 : -5, 0});
    registry.addComponent(projectile, ENTITY_HITBOX);
    registry.addComponent(projectile, Projectile{fromPlayer});
}
