/*
** EPITECH PROJECT, 2026
** PositionSystem
** File description:
** PositionSystem
*/

#include "EngineSystems.hpp"
#include "Position.hpp"
#include "Velocity.hpp"
#include "Zipper.hpp"

#include <tuple>

void PositionSystem::operator()(Registry &registry) const {
    SparseArray<Position> &positions = registry.getComponents<Position>();
    SparseArray<Velocity> &velocities = registry.getComponents<Velocity>();

    for (std::tuple<Position &, Velocity &> components : Containers::Zipper(positions, velocities)) {
        Position &position = std::get<0>(components);
        Velocity &velocity = std::get<1>(components);

        position.x += velocity.x;
        position.y += velocity.y;
    }
}
