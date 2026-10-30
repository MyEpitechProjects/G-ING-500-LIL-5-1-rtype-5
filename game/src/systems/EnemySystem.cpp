/*
** EPITECH PROJECT, 2026
** EnemySystem
** File description:
** EnemySystem
*/

#include "GameSystems.hpp"
#include "Enemy.hpp"
#include "Position.hpp"
#include "Zipper.hpp"

#include <cstdlib>
#include <tuple>
#include <vector>

void rtype::EnemySystem::operator()(Registry &registry, unsigned int &tick, int enemyInterval, int height, const std::function<void(int)> &spawnEnemy, const std::function<void(int, int, bool)> &spawnProjectile) const {
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Enemy> &enemies = registry.getComponents<Enemy>();
    std::vector<Position> shots;

    if (tick % enemyInterval == 0) {
        const int y = 30 + std::rand() % (height - 60);

        spawnEnemy(y);
    }
    if (tick % 90 != 0)
        return;
    for (std::tuple<const Position &, const Enemy &> components : Containers::Zipper(positions, enemies)) {
        const Position position = std::get<0>(components);

        shots.push_back(Position{position.x - 20, position.y});
    }
    for (const Position &shot : shots)
        spawnProjectile(shot.x, shot.y, false);
}
