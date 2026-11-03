/*
** EPITECH PROJECT, 2026
** ShotSystem
** File description:
** ShotSystem
*/

#include "GameSystems.hpp"
#include "Controllable.hpp"
#include "CoolDown.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "Zipper.hpp"

#include <tuple>
#include <vector>

void rtype::ShotSystem::operator()(Registry &registry, double shotInterval, const std::function<void(int, int, bool)> &spawnProjectile) const {
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Player> &players = registry.getComponents<Player>();
    const SparseArray<Controllable> &controls = registry.getComponents<Controllable>();
    SparseArray<CoolDown> &cooldowns = registry.getComponents<CoolDown>();
    std::vector<Position> shots;

    for (std::tuple<const Position &, const Player &, const Controllable &, CoolDown &> components
        : Containers::Zipper(positions, players, controls, cooldowns)) {
        const Controllable &input = std::get<2>(components);
        CoolDown &cooldown = std::get<3>(components);

        if (input.fire && cooldown.shootCooldown == 0) {
            const Position position = std::get<0>(components);

            cooldown.shootCooldown = shotInterval;
            shots.push_back(Position{position.x + 20, position.y});
        }
    }
    for (const Position &shot : shots)
        spawnProjectile(static_cast<int>(shot.x), static_cast<int>(shot.y), true);
}
