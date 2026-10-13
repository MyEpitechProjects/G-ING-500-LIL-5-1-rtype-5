#ifndef GAME_SYSTEMS_HPP
    #define GAME_SYSTEMS_HPP

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <limits>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "Components.hpp"
#include "Entity.hpp"
#include "GameMessage.hpp"
#include "Registry.hpp"

namespace rtype {
    class PositionSystem {
        public:
            void operator()(Registry &registry) const {
                SparseArray<Position> &positions = registry.get_components<Position>();
                SparseArray<Velocity> &velocities = registry.get_components<Velocity>();
                for (std::size_t i = 0; i < positions.size() && i < velocities.size(); i++) {
                    std::optional<Position> &position = positions[i];
                    std::optional<Velocity> &velocity = velocities[i];
                    if (position && velocity) {
                        position->x += velocity->x;
                        position->y += velocity->y;
                    }
                }
            }
    };
    

    class ControlAssignmentSystem {
        public:
            void operator()(Registry &registry, Entity player, const Controllable &controls) const {
                registry.get_components<Controllable>().insert_at(player.id(), controls);
            }
    };

    class InputQueueSystem {
        public:
            void operator()(std::vector<GameInput> &queuedInput, std::mutex &inputMutex, Registry &registry, const std::function<std::optional<Entity>(int)> &findPlayer, const std::function<void(int)> &addPlayer, const std::function<void(Entity)> &removePlayer) const {
                std::vector<GameInput> input;
                {
                    std::lock_guard<std::mutex> lock(inputMutex);
                    input.swap(queuedInput);
                }
                for (const GameInput &message : input) {
                    if (message.playerId > static_cast<std::size_t>(std::numeric_limits<int>::max()))
                        continue;
                    const int playerId = static_cast<int>(message.playerId);
                    const std::optional<Entity> player = findPlayer(playerId);
                    if (message.type == GameInputType::AddPlayer) {
                        if (!player)
                            addPlayer(playerId);
                    } else if (message.type == GameInputType::RemovePlayer) {
                        if (player)
                            removePlayer(*player);
                    } else if (message.type == GameInputType::SetControls && player) {
                        ControlAssignmentSystem{}(registry, *player, message.controls);
                    }
                }
            }
    };

    class SnapshotSystem {
        public:
            void operator()(const Registry &registry, const std::vector<Entity> &entities, unsigned int tick, unsigned int score, std::vector<GameOutput> &outputQueue, std::mutex &outputMutex) const {
                const SparseArray<Position> &positions = registry.get_components<Position>();
                const SparseArray<Health> &health = registry.get_components<Health>();
                const SparseArray<Player> &players = registry.get_components<Player>();
                const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
                const SparseArray<Projectile> &projectiles = registry.get_components<Projectile>();
                GameOutput output{};
                output.type = GameOutputType::Snapshot;
                output.tick = tick;
                output.score = score;
                output.entities.reserve(entities.size());
                for (const Entity &entity : entities) {
                    const std::size_t id = entity.id();
                    if (id >= positions.size() || !positions[id])
                        continue;
                    GameEntityKind kind;
                    if (id < players.size() && players[id])
                        kind = GameEntityKind::Player;
                    else if (id < enemies.size() && enemies[id])
                        kind = GameEntityKind::Enemy;
                    else if (id < projectiles.size() && projectiles[id])
                        kind = GameEntityKind::Projectile;
                    else
                        continue;
                    const int entityHealth = id < health.size() && health[id] ? health[id]->value : 0;
                    output.entities.push_back(GameEntityState{id, kind, *positions[id], entityHealth});
                }
                std::lock_guard<std::mutex> lock(outputMutex);
                outputQueue.push_back(std::move(output));
            }
    };

    class PlayerSpawnSystem {
        public:
            void operator()(Registry &registry, Entity player, int playerId) const {
                const std::size_t id = player.id();
                registry.get_components<Position>().insert_at(id, Position{60, 300});
                registry.get_components<Velocity>().insert_at(id, Velocity{0, 0});
                registry.get_components<Health>().insert_at(id, Health{3});
                registry.get_components<Controllable>().insert_at(id, Controllable{false, false, false, false, false});
                registry.get_components<Player>().insert_at(id, Player{playerId});
                registry.get_components<CoolDown>().insert_at(id, CoolDown{0});
            }
    };

    class PlayerRemovalSystem {
        public:
            void operator()(Registry &registry, Entity player, const std::function<void(Entity)> &destroyEntity) const {
                const SparseArray<Player> &players = registry.get_components<Player>();
                if (player.id() < players.size() && players[player.id()])
                    destroyEntity(player);
            }
    };

    class EnemySpawnSystem {
        public:
            void operator()(Registry &registry, Entity enemy, int x, int y) const {
                const std::size_t id = enemy.id();
                registry.get_components<Position>().insert_at(id, Position{x, y});
                registry.get_components<Velocity>().insert_at(id, Velocity{-2, 0});
                registry.get_components<Health>().insert_at(id, Health{1});
                registry.get_components<Enemy>().insert_at(id, Enemy{});
            }
    };

    class ProjectileSpawnSystem {
        public:
            void operator()(Registry &registry, Entity projectile, int x, int y, bool fromPlayer) const {
                const std::size_t id = projectile.id();
                registry.get_components<Position>().insert_at(id, Position{x, y});
                registry.get_components<Velocity>().insert_at(id, Velocity{fromPlayer ? 9 : -5, 0});
                registry.get_components<Projectile>().insert_at(id, Projectile{fromPlayer});
            }
    };

    class CooldownSystem {
        public:
            void operator()(Registry &registry, double elapsedSeconds) const {
                SparseArray<CoolDown> &cooldowns = registry.get_components<CoolDown>();
                for (SparseArray<CoolDown>::value_type &cooldown : cooldowns) {
                    if (cooldown && cooldown->shootCooldown > 0)
                        cooldown->shootCooldown = std::max(0.0, cooldown->shootCooldown - elapsedSeconds);
                }
            }
    };

    class ControlSystem {
        public:
            void operator()(Registry &registry, const std::vector<Entity> &entities, int playerSpeed) const {
                SparseArray<Velocity> &velocities = registry.get_components<Velocity>();
                const SparseArray<Player> &players = registry.get_components<Player>();
                const SparseArray<Controllable> &controls = registry.get_components<Controllable>();
                for (const Entity &player : entities) {
                    const std::size_t id = player.id();
                    if (id >= players.size() || !players[id] || id >= controls.size() || !controls[id])
                        continue;
                    const Controllable &input = *controls[id];
                    velocities.insert_at(id, Velocity{
                        (static_cast<int>(input.right) - static_cast<int>(input.left)) * playerSpeed,
                        (static_cast<int>(input.down) - static_cast<int>(input.up)) * playerSpeed
                    });
                }
            }
    };

    class ShotSystem {
        public:
            void operator()(Registry &registry, const std::vector<Entity> &entities, int shotInterval, const std::function<void(int, int, bool)> &spawnProjectile) const {
                const SparseArray<Position> &positions = registry.get_components<Position>();
                const SparseArray<Player> &players = registry.get_components<Player>();
                const SparseArray<Controllable> &controls = registry.get_components<Controllable>();
                SparseArray<CoolDown> &cooldowns = registry.get_components<CoolDown>();
                for (const Entity &player : entities) {
                    const std::size_t id = player.id();
                    if (id >= players.size() || !players[id] || id >= controls.size() || !controls[id]
                        || id >= positions.size() || !positions[id] || id >= cooldowns.size() || !cooldowns[id])
                        continue;
                    if (controls[id]->fire && cooldowns[id]->shootCooldown == 0) {
                        spawnProjectile(positions[id]->x + 20, positions[id]->y, true);
                        cooldowns[id]->shootCooldown = shotInterval;
                    }
                }
            }
    };

    class EnemySystem {
        public:
            void operator()(Registry &registry, const std::vector<Entity> &entities, unsigned int &tick, int enemyInterval, int height, const std::function<void(int)> &spawnEnemy, const std::function<void(int, int, bool)> &spawnProjectile) const {
                ++tick;
                if (tick % enemyInterval == 0) {
                    const int y = 30 + static_cast<int>((tick / enemyInterval * 97) % (height - 60));
                    spawnEnemy(y);
                }
                if (tick % 90 != 0)
                    return;
                const SparseArray<Position> &positions = registry.get_components<Position>();
                const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
                for (const Entity &enemy : entities) {
                    const std::size_t id = enemy.id();
                    if (id < enemies.size() && enemies[id] && id < positions.size() && positions[id])
                        spawnProjectile(positions[id]->x - 20, positions[id]->y, false);
                }
            }
    };

    class BoundsSystem {
        public:
            void operator()(Registry &registry, const std::vector<Entity> &entities, int width, int height, const std::function<void(Entity)> &destroyEntity) const {
                SparseArray<Position> &positions = registry.get_components<Position>();
                const SparseArray<Player> &players = registry.get_components<Player>();
                for (const Entity &entity : entities) {
                    const std::size_t id = entity.id();
                    if (id >= positions.size() || !positions[id])
                        continue;
                    if (id < players.size() && players[id]) {
                        positions[id]->x = std::clamp(positions[id]->x, 0, width - 1);
                        positions[id]->y = std::clamp(positions[id]->y, 0, height - 1);
                    } else if (positions[id]->x < -20 || positions[id]->x > width + 20
                        || positions[id]->y < -20 || positions[id]->y > height + 20) {
                        destroyEntity(entity);
                    }
                }
            }
    };

    inline bool overlaps(const Position &left, const Position &right) {
        return std::abs(left.x - right.x) <= 16 && std::abs(left.y - right.y) <= 12;
    }

    class ProjectileCollisionSystem {
        public:
            void operator()(Registry &registry, const std::vector<Entity> &entities, unsigned int &score, const std::function<void(Entity)> &destroyEntity) const {
                const SparseArray<Position> &positions = registry.get_components<Position>();
                SparseArray<Health> &health = registry.get_components<Health>();
                const SparseArray<Player> &players = registry.get_components<Player>();
                const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
                const SparseArray<Projectile> &projectiles = registry.get_components<Projectile>();
                for (const Entity &shot : entities) {
                    const std::size_t shotId = shot.id();
                    if (shotId >= projectiles.size() || !projectiles[shotId]
                        || shotId >= positions.size() || !positions[shotId])
                        continue;
                    const bool fromPlayer = projectiles[shotId]->fromPlayer;
                    for (const Entity &target : entities) {
                        const std::size_t targetId = target.id();
                        if (shotId == targetId || targetId >= positions.size() || !positions[targetId]
                            || targetId >= health.size() || !health[targetId])
                            continue;
                        if (fromPlayer ? (targetId >= enemies.size() || !enemies[targetId])
                            : (targetId >= players.size() || !players[targetId]))
                            continue;
                        if (!overlaps(*positions[shotId], *positions[targetId]))
                            continue;
                        destroyEntity(shot);
                        if (--health[targetId]->value <= 0) {
                            if (fromPlayer)
                                score += 100;
                            destroyEntity(target);
                        }
                        break;
                    }
                }
            }
    };

    class EnemyCollisionSystem {
        public:
            void operator()(Registry &registry, const std::vector<Entity> &entities,
                const std::function<void(Entity)> &destroyEntity) const {
                const SparseArray<Position> &positions = registry.get_components<Position>();
                SparseArray<Health> &health = registry.get_components<Health>();
                const SparseArray<Player> &players = registry.get_components<Player>();
                const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
                for (const Entity &enemy : entities) {
                    const std::size_t enemyId = enemy.id();
                    if (enemyId >= enemies.size() || !enemies[enemyId]
                        || enemyId >= positions.size() || !positions[enemyId])
                        continue;
                    for (const Entity &player : entities) {
                        const std::size_t playerId = player.id();
                        if (playerId >= players.size() || !players[playerId]
                            || playerId >= positions.size() || !positions[playerId]
                            || playerId >= health.size() || !health[playerId])
                            continue;
                        if (overlaps(*positions[enemyId], *positions[playerId])) {
                            destroyEntity(enemy);
                            if (--health[playerId]->value <= 0)
                                destroyEntity(player);
                            break;
                        }
                    }
                }
            }
    };
}

#endif
