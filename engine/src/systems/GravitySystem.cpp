/*
** EPITECH PROJECT, 2026
** GravitySystem
** File description:
** GravitySystem
*/

#include "EngineSystems.hpp"
#include "Velocity.hpp"
#include "Zipper.hpp"

#include <tuple>

void GravitySystem::operator()(Registry &registry, float gravity) const {
    SparseArray<Velocity> &velocities = registry.getComponents<Velocity>();

    for (std::tuple<Velocity &> components : Containers::Zipper(velocities))
        std::get<0>(components).y += gravity;
}
