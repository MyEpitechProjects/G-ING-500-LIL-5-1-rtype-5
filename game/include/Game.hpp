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
            double shotInterval = 0.3;
            int enemyInterval = 60;

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

            Entity createEntity();
            void destroyEntity(Entity entity);
            Entity spawnEnemy(int y);
            Entity spawnProjectile(int x, int y, bool fromPlayer);
            std::optional<Entity> getEntityFromPlayerId(int playerId) const;
            std::size_t getPlayerCount() const;
            Entity addPlayer(int id);
            void removePlayer(Entity player);

        public:
            Game(std::shared_ptr<WakeSignal> &wakeSignal);
            ~Game() = default;

            Registry &getRegistry();
            const std::vector<Entity> &getEntities() const;
            unsigned int getScore() const;
            unsigned int getTick() const;

            std::vector<rtype::GameOutput> takeOutputQueue();

            void addInputQueue(const std::vector<rtype::GameInput> &input);
            void init();
            void stop();
            void update();
            void run();
    };
}

#endif
