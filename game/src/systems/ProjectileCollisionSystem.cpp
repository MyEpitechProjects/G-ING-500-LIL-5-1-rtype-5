/*
** EPITECH PROJECT, 2026
** ProjectileCollisionSystem
** File description:
** ProjectileCollisionSystem
*/

#include "GameSystems.hpp"
#include "Enemy.hpp"
#include "Health.hpp"
#include "Hitbox.hpp"
#include "Player.hpp"
#include "Position.hpp"
#include "Projectile.hpp"
#include "Zipper.hpp"

#include <tuple>
#include <utility>
#include <vector>

void rtype::ProjectileCollisionSystem::operator()(Registry &registry, unsigned int &score, const std::function<void(Entity)> &destroyEntity) const {
    using ShotZip = Containers::Zipper<const SparseArray<Position>, const SparseArray<Hitbox>, const SparseArray<Projectile>>;
    using TargetZip = Containers::Zipper<const SparseArray<Position>, const SparseArray<Hitbox>, SparseArray<Health>>;
    const SparseArray<Position> &positions = registry.getComponents<Position>();
    const SparseArray<Hitbox> &hitboxes = registry.getComponents<Hitbox>();
    SparseArray<Health> &health = registry.getComponents<Health>();
    const SparseArray<Projectile> &projectiles = registry.getComponents<Projectile>();
    ShotZip shots(positions, hitboxes, projectiles);
    TargetZip targets(positions, hitboxes, health);
    std::vector<std::pair<Entity, bool>> toDestroy;

    for (ShotZip::iterator shotIt = shots.begin(); shotIt != shots.end(); ++shotIt) {
        const std::size_t shotId = shotIt.getIndex();
        std::tuple<const Position &, const Hitbox &, const Projectile &> shot = *shotIt;
        const bool fromPlayer = std::get<2>(shot).fromPlayer;

        for (TargetZip::iterator targetIt = targets.begin(); targetIt != targets.end(); ++targetIt) {
            const Entity targetEntity(targetIt.getIndex());
            std::tuple<const Position &, const Hitbox &, Health &> target = *targetIt;
            Health &targetHealth = std::get<2>(target);

            if (shotId == targetEntity.getId() || targetHealth.value <= 0)
                continue;
            if (fromPlayer ? !registry.hasComponent<Enemy>(targetEntity)
                : !registry.hasComponent<Player>(targetEntity))
                continue;
            if (!overlaps(std::get<0>(shot), std::get<1>(shot), std::get<0>(target), std::get<1>(target)))
                continue;
            toDestroy.emplace_back(Entity(shotId), false);
            if (--targetHealth.value <= 0)
                toDestroy.emplace_back(targetEntity, fromPlayer);
            break;
        }
    }
    for (const std::pair<Entity, bool> &entry : toDestroy) {
        if (entry.second)
            score += 100;
        destroyEntity(entry.first);
    }
}
