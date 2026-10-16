#include "GameSystems.hpp"
#include "Zipper.hpp"

namespace rtype {
    void PositionSystem::operator()(Registry &registry) const {
        SparseArray<Position> &positions = registry.get_components<Position>();
        SparseArray<Velocity> &velocities = registry.get_components<Velocity>();
        for (std::tuple<Position &, Velocity &> components : containers::zipper(positions, velocities)) {
            Position &position = std::get<0>(components);
            Velocity &velocity = std::get<1>(components);
            position.x += velocity.x;
            position.y += velocity.y;
        }
    }

    void ControlAssignmentSystem::operator()(Registry &registry, Entity player, const Controllable &controls) const {
        registry.get_components<Controllable>().insert_at(player.id(), controls);
    }

    void InputQueueSystem::operator()(std::vector<GameInput> &queuedInput, std::mutex &inputMutex, Registry &registry, const std::function<std::optional<Entity>(int)> &findPlayer, const std::function<void(int)> &addPlayer, const std::function<void(Entity)> &removePlayer) const {
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

    void SnapshotSystem::operator()(const Registry &registry, const std::vector<Entity> &entities, unsigned int tick, unsigned int score, std::vector<GameOutput> &outputQueue, std::mutex &outputMutex) const {
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
            const bool fromPlayer = kind == GameEntityKind::Projectile && projectiles[id]->fromPlayer;
            output.entities.push_back(GameEntityState{id, kind, *positions[id], entityHealth, fromPlayer});
        }
        std::lock_guard<std::mutex> lock(outputMutex);
        outputQueue.push_back(std::move(output));
    }

    void PlayerSpawnSystem::operator()(Registry &registry, Entity player, int playerId) const {
        const std::size_t id = player.id();
        registry.get_components<Position>().insert_at(id, Position{60, 300});
        registry.get_components<Velocity>().insert_at(id, Velocity{0, 0});
        registry.get_components<Health>().insert_at(id, Health{3});
        registry.get_components<Controllable>().insert_at(id, Controllable{false, false, false, false, false});
        registry.get_components<Player>().insert_at(id, Player{playerId});
        registry.get_components<CoolDown>().insert_at(id, CoolDown{0});
    }

    void PlayerRemovalSystem::operator()(Registry &registry, Entity player, const std::function<void(Entity)> &destroyEntity) const {
        const SparseArray<Player> &players = registry.get_components<Player>();
        if (player.id() < players.size() && players[player.id()])
            destroyEntity(player);
    }

    void EnemySpawnSystem::operator()(Registry &registry, Entity enemy, int x, int y) const {
        const std::size_t id = enemy.id();
        registry.get_components<Position>().insert_at(id, Position{x, y});
        registry.get_components<Velocity>().insert_at(id, Velocity{-2, 0});
        registry.get_components<Health>().insert_at(id, Health{1});
        registry.get_components<Enemy>().insert_at(id, Enemy{});
    }

    void ProjectileSpawnSystem::operator()(Registry &registry, Entity projectile, int x, int y, bool fromPlayer) const {
        const std::size_t id = projectile.id();
        registry.get_components<Position>().insert_at(id, Position{x, y});
        registry.get_components<Velocity>().insert_at(id, Velocity{fromPlayer ? 13 : -5, 0});
        registry.get_components<Projectile>().insert_at(id, Projectile{fromPlayer});
    }

    void CooldownSystem::operator()(Registry &registry, double elapsedSeconds) const {
        SparseArray<CoolDown> &cooldowns = registry.get_components<CoolDown>();
        for (std::tuple<CoolDown &> components : containers::zipper(cooldowns)) {
            CoolDown &cooldown = std::get<0>(components);
            if (cooldown.shootCooldown > 0)
                cooldown.shootCooldown = std::max(0.0, cooldown.shootCooldown - elapsedSeconds);
        }
    }

    void ControlSystem::operator()(Registry &registry, int playerSpeed) const {
        SparseArray<Velocity> &velocities = registry.get_components<Velocity>();
        const SparseArray<Player> &players = registry.get_components<Player>();
        const SparseArray<Controllable> &controls = registry.get_components<Controllable>();
        containers::zipper<const SparseArray<Player>, const SparseArray<Controllable>> zipped(players, controls);
        for (containers::zipper<const SparseArray<Player>, const SparseArray<Controllable>>::iterator it = zipped.begin(); it != zipped.end(); ++it) {
            const std::size_t id = it.index();
            const Controllable &input = std::get<1>(*it);
            velocities.insert_at(id, Velocity{
                (static_cast<int>(input.right) - static_cast<int>(input.left)) * playerSpeed,
                (static_cast<int>(input.down) - static_cast<int>(input.up)) * playerSpeed
            });
        }
    }

    void ShotSystem::operator()(Registry &registry, double shotInterval, const std::function<void(int, int, bool)> &spawnProjectile) const {
        const SparseArray<Position> &positions = registry.get_components<Position>();
        const SparseArray<Player> &players = registry.get_components<Player>();
        const SparseArray<Controllable> &controls = registry.get_components<Controllable>();
        SparseArray<CoolDown> &cooldowns = registry.get_components<CoolDown>();
        std::vector<Position> shots;
        for (std::tuple<const Position &, const Player &, const Controllable &, CoolDown &> components
            : containers::zipper(positions, players, controls, cooldowns)) {
            const Controllable &input = std::get<2>(components);
            CoolDown &cooldown = std::get<3>(components);
            if (input.fire && cooldown.shootCooldown == 0) {
                const Position position = std::get<0>(components);
                cooldown.shootCooldown = shotInterval;
                shots.push_back(Position{position.x + 20, position.y});
            }
        }
        for (const Position &shot : shots)
            spawnProjectile(shot.x, shot.y, true);
    }

    void EnemySystem::operator()(Registry &registry, unsigned int &tick, int enemyInterval, int height, const std::function<void(int)> &spawnEnemy, const std::function<void(int, int, bool)> &spawnProjectile) const {
        if (tick % enemyInterval == 0) {
            const int y = 30 + std::rand() % (height - 60);
            spawnEnemy(y);
        }
        if (tick % 90 != 0)
            return;
        const SparseArray<Position> &positions = registry.get_components<Position>();
        const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
        std::vector<Position> shots;
        for (std::tuple<const Position &, const Enemy &> components : containers::zipper(positions, enemies)) {
            const Position position = std::get<0>(components);
            shots.push_back(Position{position.x - 20, position.y});
        }
        for (const Position &shot : shots)
            spawnProjectile(shot.x, shot.y, false);
    }

    void BoundsSystem::operator()(Registry &registry, int width, int height, const std::function<void(Entity)> &destroyEntity) const {
        SparseArray<Position> &positions = registry.get_components<Position>();
        const SparseArray<Player> &players = registry.get_components<Player>();
        using PositionZip = containers::zipper<SparseArray<Position>>;
        PositionZip zipped(positions);
        std::vector<Entity> toDestroy;
        for (PositionZip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
            const std::size_t id = it.index();
            Position &position = std::get<0>(*it);
            if (id < players.size() && players[id]) {
                position.x = std::clamp(position.x, 0, width - 1);
                position.y = std::clamp(position.y, 0, height - 1);
            } else if (position.x < -20 || position.x > width + 20
                || position.y < -20 || position.y > height + 20) {
                toDestroy.emplace_back(id);
            }
        }
        for (const Entity &entity : toDestroy)
            destroyEntity(entity);
    }

    bool overlaps(const Position &left, const Position &right) {
        return std::abs(left.x - right.x) <= 16 && std::abs(left.y - right.y) <= 12;
    }

    void ProjectileCollisionSystem::operator()(Registry &registry, unsigned int &score, const std::function<void(Entity)> &destroyEntity) const {
        const SparseArray<Position> &positions = registry.get_components<Position>();
        SparseArray<Health> &health = registry.get_components<Health>();
        const SparseArray<Player> &players = registry.get_components<Player>();
        const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
        const SparseArray<Projectile> &projectiles = registry.get_components<Projectile>();
        using ShotZip = containers::zipper<const SparseArray<Position>, const SparseArray<Projectile>>;
        using TargetZip = containers::zipper<const SparseArray<Position>, SparseArray<Health>>;
        ShotZip shots(positions, projectiles);
        TargetZip targets(positions, health);
        std::vector<std::pair<Entity, bool>> toDestroy;
        for (ShotZip::iterator shotIt = shots.begin(); shotIt != shots.end(); ++shotIt) {
            const std::size_t shotId = shotIt.index();
            std::tuple<const Position &, const Projectile &> shot = *shotIt;
            const bool fromPlayer = std::get<1>(shot).fromPlayer;
            for (TargetZip::iterator targetIt = targets.begin(); targetIt != targets.end(); ++targetIt) {
                const std::size_t targetId = targetIt.index();
                std::tuple<const Position &, Health &> target = *targetIt;
                Health &targetHealth = std::get<1>(target);
                if (shotId == targetId || targetHealth.value <= 0)
                    continue;
                if (fromPlayer ? (targetId >= enemies.size() || !enemies[targetId])
                    : (targetId >= players.size() || !players[targetId]))
                    continue;
                if (!overlaps(std::get<0>(shot), std::get<0>(target)))
                    continue;
                toDestroy.emplace_back(Entity(shotId), false);
                if (--targetHealth.value <= 0)
                    toDestroy.emplace_back(Entity(targetId), fromPlayer);
                break;
            }
        }
        for (const std::pair<Entity, bool> &entry : toDestroy) {
            if (entry.second)
                score += 100;
            destroyEntity(entry.first);
        }
    }

    void EnemyCollisionSystem::operator()(Registry &registry, const std::function<void(Entity)> &destroyEntity) const {
        const SparseArray<Position> &positions = registry.get_components<Position>();
        SparseArray<Health> &health = registry.get_components<Health>();
        const SparseArray<Player> &players = registry.get_components<Player>();
        const SparseArray<Enemy> &enemies = registry.get_components<Enemy>();
        std::vector<Entity> toDestroy;

        using EnemyZip = containers::zipper<const SparseArray<Position>, const SparseArray<Enemy>>;
        using PlayerZip = containers::zipper<const SparseArray<Position>, const SparseArray<Player>, SparseArray<Health>>;
        EnemyZip enemyComponents(positions, enemies);
        PlayerZip playerComponents(positions, players, health);
        for (EnemyZip::iterator enemyIt = enemyComponents.begin(); enemyIt != enemyComponents.end(); ++enemyIt) {
            const std::size_t enemyId = enemyIt.index();
            const Position &enemyPosition = std::get<0>(*enemyIt);
            for (PlayerZip::iterator playerIt = playerComponents.begin(); playerIt != playerComponents.end(); ++playerIt) {
                const std::size_t playerId = playerIt.index();
                std::tuple<const Position &, const Player &, Health &> player = *playerIt;
                if (std::get<2>(player).value <= 0)
                    continue;
                if (overlaps(enemyPosition, std::get<0>(player))) {
                    toDestroy.emplace_back(enemyId);
                    if (--std::get<2>(player).value <= 0)
                        toDestroy.emplace_back(playerId);
                    break;
                }
            }
        }
        for (const Entity &entity : toDestroy)
            destroyEntity(entity);
    }
}
