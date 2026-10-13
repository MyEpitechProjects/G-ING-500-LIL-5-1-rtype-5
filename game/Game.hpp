#ifndef GAME_HPP
    #define GAME_HPP

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include "Components.hpp"
#include "Entity.hpp"
#include "Registry.hpp"
#include "GameMessage.hpp"
#include "GameSystems.hpp"
#include "WakeSignal.hpp"

namespace rtype {
    class Game {
        private:
            int width = 800;
            int height = 600;
            int playerSpeed = 5;
            int shotInterval = 2;
            int enemyInterval = 15;

            bool _running = false;
            Registry _registry;
            std::size_t _nextId = 0;
            std::vector<Entity> _entities;
            unsigned int _tick = 0;
            unsigned int _score = 0;
            std::chrono::steady_clock::time_point _lastUpdate = std::chrono::steady_clock::now();

            std::vector<rtype::GameInput> _in;
            std::vector<rtype::GameOutput> _out;
            std::mutex _inMutex;
            std::mutex _outMutex;
            std::shared_ptr<WakeSignal> _wakeSignal;

            Entity createEntity() {
                Entity entity(_nextId++);
                _entities.push_back(entity);
                return entity;
            }

            void destroyEntity(Entity entity) {
                std::vector<Entity>::iterator it = std::find_if(_entities.begin(), _entities.end(),
                    [entity](const Entity &current) {
                        return current.id() == entity.id();
                    }
                );
                if (it == _entities.end())
                    return;
                rtype::GameOutput output{};
                output.type = rtype::GameOutputType::EntityDestroyed;
                output.entityId = entity.id();
                output.tick = _tick;
                output.score = _score;
                {
                    std::lock_guard<std::mutex> lock(_outMutex);
                    _out.push_back(output);
                }
                _registry.erase_entity(entity);
                _entities.erase(it);
            }

            Entity spawnEnemy(int y) {
                Entity enemy = createEntity();
                EnemySpawnSystem{}(_registry, enemy, width - 20, y);
                return enemy;
            }

            Entity spawnProjectile(int x, int y, bool fromPlayer) {
                Entity projectile = createEntity();
                ProjectileSpawnSystem{}(_registry, projectile, x, y, fromPlayer);
                return projectile;
            }

            std::optional<Entity> getEntityFromPlayerId(int playerId) const {
                const SparseArray<Player> &players = _registry.get_components<Player>();
                for (const Entity &entity : _entities) {
                    if (entity.id() < players.size() && players[entity.id()]
                        && players[entity.id()]->id == playerId)
                        return entity;
                }
                return std::nullopt;
            }

            std::size_t getPlayerCount() const {
                const SparseArray<Player> &players = _registry.get_components<Player>();
                std::size_t count = 0;
                for (const Entity &entity : _entities) {
                    if (entity.id() < players.size() && players[entity.id()])
                        ++count;
                }
                return count;
            }

            Entity addPlayer(int id) {
                Entity player = createEntity();
                PlayerSpawnSystem{}(_registry, player, id);
                rtype::GameOutput output{};
                output.type = rtype::GameOutputType::PlayerCreated;
                output.clientId = static_cast<std::uint32_t>(id);
                output.entityId = player.id();
                output.tick = _tick;
                output.score = _score;
                std::lock_guard<std::mutex> lock(_outMutex);
                _out.push_back(output);
                return player;
            }

            void removePlayer(Entity player) {
                PlayerRemovalSystem{}(_registry, player,
                    [this](Entity entity) { destroyEntity(entity); }
                );
            }

        public:
            Game(std::shared_ptr<WakeSignal> &wakeSignal) : _wakeSignal(wakeSignal) { init(); }
            ~Game() = default;

            Registry &getRegistry() { return _registry; }
            const std::vector<Entity> &getEntities() const { return _entities; }
            unsigned int getScore() const { return _score; }
            unsigned int getTick() const { return _tick; }
            bool isRunning() const { return _running; }

            std::vector<rtype::GameOutput> takeOutputQueue() {
                std::lock_guard<std::mutex> lock(_outMutex);
                std::vector<rtype::GameOutput> output;
                output.swap(_out);
                return output;
            }

            void addInputQueue(const std::vector<rtype::GameInput> &input) {
                std::lock_guard<std::mutex> lock(_inMutex);
                _in.insert(_in.end(), input.begin(), input.end());
            }

            void init() {
                _registry.register_component<Position>();
                _registry.register_component<Velocity>();
                _registry.register_component<Health>();
                _registry.register_component<Controllable>();
                _registry.register_component<Player>();
                _registry.register_component<Enemy>();
                _registry.register_component<Projectile>();
                _registry.register_component<CoolDown>();
                _running = true;
            }

            void stop() { _running = false; }

            void update() {
                InputQueueSystem{}(_in, _inMutex, _registry,
                    [this](int playerId) { return getEntityFromPlayerId(playerId); },
                    [this](int playerId) { addPlayer(playerId); },
                    [this](Entity player) { removePlayer(player); }
                );
                const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
                const double elapsedSeconds = std::chrono::duration<double>(now - _lastUpdate).count();
                _lastUpdate = now;

                CooldownSystem{}(_registry, elapsedSeconds);
                ControlSystem{}(_registry, _entities, playerSpeed);
                ShotSystem{}(_registry, _entities, shotInterval,
                    [this](int x, int y, bool fromPlayer) { spawnProjectile(x, y, fromPlayer); }
                );
                EnemySystem{}(_registry, _entities, _tick, enemyInterval, height,
                    [this](int y) { spawnEnemy(y); },
                    [this](int x, int y, bool fromPlayer) { spawnProjectile(x, y, fromPlayer); }
                );
                PositionSystem{}(_registry);
                BoundsSystem{}(_registry, _entities, width, height,
                    [this](Entity entity) { destroyEntity(entity); }
                );
                ProjectileCollisionSystem{}(_registry, _entities, _score,
                    [this](Entity entity) { destroyEntity(entity); }
                );
                EnemyCollisionSystem{}(_registry, _entities,
                    [this](Entity entity) { destroyEntity(entity); }
                );
                SnapshotSystem{}(_registry, _entities, _tick, _score, _out, _outMutex);
                _wakeSignal->notify();
            }

            void run() {
                const std::chrono::nanoseconds tick(1000000000 / 60);
                std::chrono::steady_clock::time_point nextTick = std::chrono::steady_clock::now();

                while (_running) {
                    update();
                    nextTick += tick;
                    const std::chrono::steady_clock::time_point now =
                        std::chrono::steady_clock::now();
                    if (nextTick > now)
                        std::this_thread::sleep_until(nextTick);
                    else
                        nextTick = now;
                }
            }
    };
}

#endif
