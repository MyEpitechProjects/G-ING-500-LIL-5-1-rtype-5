/*
** EPITECH PROJECT, 2026
** PlayerSpawnSystem
** File description:
** PlayerSpawnSystem
*/

#include "GameSystems.hpp"
#include "Controllable.hpp"
#include "CoolDown.hpp"
#include "EntityHitbox.hpp"
#include "Health.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "Velocity.hpp"

void rtype::PlayerSpawnSystem::operator()(Registry &registry, Entity player, int playerId) const {
    registry.addComponent(player, Position{60, 300});
    registry.addComponent(player, Velocity{0, 0});
    registry.addComponent(player, ENTITY_HITBOX);
    registry.addComponent(player, Health{3});
    registry.addComponent(player, Controllable{false, false, false, false, false});
    registry.addComponent(player, Player{playerId});
    registry.addComponent(player, CoolDown{0});
}
