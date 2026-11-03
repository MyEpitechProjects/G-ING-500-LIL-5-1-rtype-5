/*
** EPITECH PROJECT, 2026
** CollisionSystem
** File description:
** CollisionSystem
*/

#include "EngineSystems.hpp"
#include "Hitbox.hpp"
#include "Position.hpp"
#include "Zipper.hpp"

#include <cstddef>
#include <tuple>
#include <vector>

void CollisionSystem::operator()(Registry &registry, const std::function<void(Entity, Entity)> &onCollision) const {
    using BodyZip = Containers::Zipper<const SparseArray<Position>, const SparseArray<Hitbox>>;
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Hitbox> &hitboxes = registry.getComponents<Hitbox>();
    BodyZip zipped(positions, hitboxes);
    std::vector<std::tuple<Entity, Position, Hitbox>> bodies;

    for (BodyZip::iterator it = zipped.begin(); it != zipped.end(); ++it)
        bodies.emplace_back(Entity(it.getIndex()), std::get<0>(*it), std::get<1>(*it));
    for (std::size_t first = 0; first < bodies.size(); first++)
        for (std::size_t second = first + 1; second < bodies.size(); second++)
            if (overlaps(std::get<1>(bodies[first]), std::get<2>(bodies[first]), std::get<1>(bodies[second]), std::get<2>(bodies[second])))
                onCollision(std::get<0>(bodies[first]), std::get<0>(bodies[second]));
}
