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
    bool overlaps(const Position &left, const Position &right);

    class PositionSystem {
        public:
            void operator()(Registry &registry) const;
    };
    
    class ControlAssignmentSystem {
        public:
            void operator()(Registry &registry, Entity player, const Controllable &controls) const;
    };

    class InputQueueSystem {
        public:
            void operator()(std::vector<GameInput> &queuedInput, std::mutex &inputMutex, Registry &registry, const std::function<std::optional<Entity>(int)> &findPlayer, const std::function<void(int)> &addPlayer, const std::function<void(Entity)> &removePlayer) const;
    };

    class SnapshotSystem {
        public:
            void operator()(const Registry &registry, const std::vector<Entity> &entities, unsigned int tick, unsigned int score, std::vector<GameOutput> &outputQueue, std::mutex &outputMutex) const;
    };

    class PlayerSpawnSystem {
        public:
            void operator()(Registry &registry, Entity player, int playerId) const;
    };

    class PlayerRemovalSystem {
        public:
            void operator()(Registry &registry, Entity player, const std::function<void(Entity)> &destroyEntity) const;
    };

    class EnemySpawnSystem {
        public:
            void operator()(Registry &registry, Entity enemy, int x, int y) const;
    };

    class ProjectileSpawnSystem {
        public:
            void operator()(Registry &registry, Entity projectile, int x, int y, bool fromPlayer) const;
    };

    class CooldownSystem {
        public:
            void operator()(Registry &registry, double elapsedSeconds) const;
    };

    class ControlSystem {
        public:
            void operator()(Registry &registry, int playerSpeed) const;
    };

    class ShotSystem {
        public:
            void operator()(Registry &registry, double shotInterval, const std::function<void(int, int, bool)> &spawnProjectile) const;
    };

    class EnemySystem {
        public:
            void operator()(Registry &registry, unsigned int &tick, int enemyInterval, int height, const std::function<void(int)> &spawnEnemy, const std::function<void(int, int, bool)> &spawnProjectile) const;
    };

    class BoundsSystem {
        public:
            void operator()(Registry &registry, int width, int height, const std::function<void(Entity)> &destroyEntity) const;
    };

    class ProjectileCollisionSystem {
        public:
            void operator()(Registry &registry, unsigned int &score, const std::function<void(Entity)> &destroyEntity) const;
    };

    class EnemyCollisionSystem {
        public:
            void operator()(Registry &registry, const std::function<void(Entity)> &destroyEntity) const;
    };
}

#endif
