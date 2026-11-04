/*
** EPITECH PROJECT, 2026
** PhysicsModule
** File description:
** PhysicsModule
*/

#include "PhysicsModule.hpp"
#include "EngineSystems.hpp"
#include "Hitbox.hpp"
#include "Position.hpp"
#include "Velocity.hpp"

#include <utility>

PhysicsModule::PhysicsModule(int left, int top, int right, int bottom, float gravity, std::function<void(Entity)> onOutOfBounds)
    : AModule("physics"), _left(left), _top(top), _right(right), _bottom(bottom), _gravity(gravity), _onOutOfBounds(std::move(onOutOfBounds)) {}

void PhysicsModule::registerComponents(Registry &registry) {
    registry.registerComponent<Position>();
    registry.registerComponent<Velocity>();
    registry.registerComponent<Hitbox>();
}

void PhysicsModule::update(Registry &registry, [[maybe_unused]] double elapsedSeconds) {
    GravitySystem{}(registry, _gravity);
    PositionSystem{}(registry);
    OutOfBoundsSystem{}(registry, _left, _top, _right, _bottom, _onOutOfBounds);
    handleCollisions(registry);
}

void PhysicsModule::handleCollisions(Registry &registry) {
    std::vector<std::pair<Entity, Entity>> collisions;

    if (_collisionHandlers.empty())
        return;
    CollisionSystem{}(registry, [&collisions](Entity left, Entity right) {
        collisions.emplace_back(left, right);
    });
    for (const std::function<void(Registry &, Entity, Entity)> &handler : _collisionHandlers)
        for (const std::pair<Entity, Entity> &collision : collisions)
            if (registry.isAlive(collision.first) && registry.isAlive(collision.second))
                handler(registry, collision.first, collision.second);
}
