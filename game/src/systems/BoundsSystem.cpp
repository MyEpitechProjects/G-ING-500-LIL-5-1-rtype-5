/*
** EPITECH PROJECT, 2026
** BoundsSystem
** File description:
** BoundsSystem
*/

#include "GameSystems.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "Zipper.hpp"

#include <algorithm>
#include <tuple>

void rtype::BoundsSystem::operator()(Registry &registry, int width, int height) const {
    SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Player> &players = registry.getComponents<Player>();

    for (std::tuple<Position &, const Player &> components : Containers::Zipper(positions, players)) {
        Position &position = std::get<0>(components);

        position.x = std::clamp(position.x, 0, width - 1);
        position.y = std::clamp(position.y, 0, height - 1);
    }
}
