/*
** EPITECH PROJECT, 2026
** GameSystems
** File description:
** GameSystems def
*/

#ifndef GAMESYSTEMS
    #define GAMESYSTEMS

    #include "Controllable.hpp"
    #include "Entity.hpp"
    #include "GameInput.hpp"
    #include "GameOutput.hpp"
    #include "Registry.hpp"
    #include <functional>
    #include <optional>
    #include <vector>

namespace rtype {
class BoundsSystem {
    public:
        void operator()(Registry &registry, int width, int height) const;
};

class ControlAssignmentSystem {
    public:
        void operator()(Registry &registry, Entity player, const Controllable &controls) const;
};

class ControlSystem {
    public:
        void operator()(Registry &registry, int playerSpeed) const;
};

class CooldownSystem {
    public:
        void operator()(Registry &registry, double elapsedSeconds) const;
};

class EnemyCollisionSystem {
    public:
        void operator()(Registry &registry, const std::function<void(Entity)> &destroyEntity) const;
};

class EnemySpawnSystem {
    public:
        void operator()(Registry &registry, Entity enemy, int x, int y) const;
};

class EnemySystem {
    public:
        void operator()(Registry &registry, unsigned int &tick, int enemyInterval, int height, const std::function<void(int)> &spawnEnemy, const std::function<void(int, int, bool)> &spawnProjectile) const;
};

class InputQueueSystem {
    public:
        void operator()(std::vector<rtype::GameInput> &queuedInput, Registry &registry, const std::function<std::optional<Entity>(int)> &findPlayer, const std::function<void(int)> &addPlayer, const std::function<void(Entity)> &removePlayer) const;
};

class PlayerRemovalSystem {
    public:
        void operator()(Registry &registry, Entity player, const std::function<void(Entity)> &destroyEntity) const;
};

class PlayerSpawnSystem {
    public:
        void operator()(Registry &registry, Entity player, int playerId) const;
};

class ProjectileCollisionSystem {
    public:
        void operator()(Registry &registry, unsigned int &score, const std::function<void(Entity)> &destroyEntity) const;
};

class ProjectileSpawnSystem {
    public:
        void operator()(Registry &registry, Entity projectile, int x, int y, bool fromPlayer) const;
};

class ShotSystem {
    public:
        void operator()(Registry &registry, double shotInterval, const std::function<void(int, int, bool)> &spawnProjectile) const;
};

class SnapshotSystem {
    public:
        void operator()(const Registry &registry, const std::vector<Entity> &entities, unsigned int tick, unsigned int score, std::vector<rtype::GameOutput> &outputQueue) const;
};
}

#endif
