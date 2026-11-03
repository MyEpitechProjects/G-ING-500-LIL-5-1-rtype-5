/*
** EPITECH PROJECT, 2026
** ControlSystem
** File description:
** ControlSystem
*/

#include "GameSystems.hpp"
#include "Controllable.hpp"
#include "Player.hpp"
#include "Velocity.hpp"
#include "Zipper.hpp"

#include <tuple>

void rtype::ControlSystem::operator()(Registry &registry, int playerSpeed) const {
    using ControlZip = Containers::Zipper<const SparseArray<Player>, const SparseArray<Controllable>>;
    SparseArray<Velocity> &velocities = registry.getComponents<Velocity>();
    const SparseArray<Player> &players = registry.getComponents<Player>();
    const SparseArray<Controllable> &controls = registry.getComponents<Controllable>();
    ControlZip zipped(players, controls);

    for (ControlZip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        const std::size_t id = it.getIndex();
        const Controllable &input = std::get<1>(*it);

        velocities.insertAt(id, Velocity{
            static_cast<float>((static_cast<int>(input.right) - static_cast<int>(input.left)) * playerSpeed),
            static_cast<float>((static_cast<int>(input.down) - static_cast<int>(input.up)) * playerSpeed)
        });
    }
}
