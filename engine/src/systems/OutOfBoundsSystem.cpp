/*
** EPITECH PROJECT, 2026
** OutOfBoundsSystem
** File description:
** OutOfBoundsSystem
*/

#include "EngineSystems.hpp"
#include "Position.hpp"
#include "Zipper.hpp"

#include <tuple>
#include <vector>

void OutOfBoundsSystem::operator()(Registry &registry, int left, int top, int right, int bottom, const std::function<void(Entity)> &onOut) const {
    using PositionZip = Containers::Zipper<const SparseArray<Position>>;
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    PositionZip zipped(positions);
    std::vector<Entity> outside;

    for (PositionZip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        const Position &position = std::get<0>(*it);

        if (position.x < left || position.x > right || position.y < top || position.y > bottom)
            outside.emplace_back(it.getIndex());
    }
    for (const Entity &entity : outside)
        onOut(entity);
}
