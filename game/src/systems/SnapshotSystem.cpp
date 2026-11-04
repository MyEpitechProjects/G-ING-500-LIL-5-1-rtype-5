/*
** EPITECH PROJECT, 2026
** SnapshotSystem
** File description:
** SnapshotSystem
*/

#include "GameSystems.hpp"
#include "Enemy.hpp"
#include "Health.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "Projectile.hpp"

#include <utility>

void rtype::SnapshotSystem::operator()(const Registry &registry, const std::vector<Entity> &entities, unsigned int tick, unsigned int score, std::vector<rtype::GameOutput> &outputQueue) const {
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Health> &health = registry.getComponents<Health>();
    const SparseArray<Projectile> &projectiles = registry.getComponents<Projectile>();
    rtype::GameOutput output{};

    output.type = rtype::GameOutputType::Snapshot;
    output.tick = tick;
    output.score = score;
    output.entities.reserve(entities.size());
    for (const Entity &entity : entities) {
        const std::size_t id = entity.getId();
        rtype::GameEntityKind kind;

        if (!registry.hasComponent<Position>(entity))
            continue;
        if (registry.hasComponent<Player>(entity))
            kind = rtype::GameEntityKind::Player;
        else if (registry.hasComponent<Enemy>(entity))
            kind = rtype::GameEntityKind::Enemy;
        else if (registry.hasComponent<Projectile>(entity))
            kind = rtype::GameEntityKind::Projectile;
        else
            continue;
        const int entityHealth = registry.hasComponent<Health>(entity) ? health[id]->value : 0;
        const bool fromPlayer = kind == rtype::GameEntityKind::Projectile && projectiles[id]->fromPlayer;

        output.entities.push_back(rtype::GameEntityState{id, kind, *positions[id], entityHealth, fromPlayer});
    }
    outputQueue.push_back(std::move(output));
}
