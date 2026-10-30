/*
** EPITECH PROJECT, 2026
** EnemyCollisionSystem
** File description:
** EnemyCollisionSystem
*/

#include "GameSystems.hpp"
#include "Enemy.hpp"
#include "Health.hpp"
#include "Hitbox.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "Zipper.hpp"

#include <tuple>
#include <vector>

void rtype::EnemyCollisionSystem::operator()(Registry &registry, const std::function<void(Entity)> &destroyEntity) const {
    using EnemyZip = Containers::Zipper<const SparseArray<Position>, const SparseArray<Hitbox>, const SparseArray<Enemy>>;
    using PlayerZip = Containers::Zipper<const SparseArray<Position>, const SparseArray<Hitbox>, const SparseArray<Player>, SparseArray<Health>>;
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Hitbox> &hitboxes = registry.getComponents<Hitbox>();
    SparseArray<Health> &health = registry.getComponents<Health>();
    const SparseArray<Player> &players = registry.getComponents<Player>();
    const SparseArray<Enemy> &enemies = registry.getComponents<Enemy>();
    EnemyZip enemyComponents(positions, hitboxes, enemies);
    PlayerZip playerComponents(positions, hitboxes, players, health);
    std::vector<Entity> toDestroy;

    for (EnemyZip::iterator enemyIt = enemyComponents.begin(); enemyIt != enemyComponents.end(); ++enemyIt) {
        const std::size_t enemyId = enemyIt.getIndex();
        const Position &enemyPosition = std::get<0>(*enemyIt);
        const Hitbox &enemyHitbox = std::get<1>(*enemyIt);

        for (PlayerZip::iterator playerIt = playerComponents.begin(); playerIt != playerComponents.end(); ++playerIt) {
            const std::size_t playerId = playerIt.getIndex();
            std::tuple<const Position &, const Hitbox &, const Player &, Health &> player = *playerIt;

            if (std::get<3>(player).value <= 0)
                continue;
            if (overlaps(enemyPosition, enemyHitbox, std::get<0>(player), std::get<1>(player))) {
                toDestroy.emplace_back(enemyId);
                if (--std::get<3>(player).value <= 0)
                    toDestroy.emplace_back(playerId);
                break;
            }
        }
    }
    for (const Entity &entity : toDestroy)
        destroyEntity(entity);
}
