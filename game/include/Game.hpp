/*
** EPITECH PROJECT, 2026
** Game
** File description:
** Game def
*/

#ifndef GAME
    #define GAME

    #include "Engine.hpp"
    #include "GameInput.hpp"
    #include "GameOutput.hpp"
    #include <chrono>
    #include <optional>
    #include <vector>

namespace rtype {
class Game {
    private:
        int _width;
        int _height;
        int _playerSpeed;
        double _shotInterval;
        int _enemyInterval;
        double _tickPeriod;

        Engine _engine;
        unsigned int _tick;
        unsigned int _score;
        std::chrono::steady_clock::time_point _lastUpdate;
        double _lag;

        std::vector<rtype::GameInput> _in;
        std::vector<rtype::GameOutput> _out;

        void destroyEntity(Entity entity);
        Entity spawnEnemy(int y);
        Entity spawnProjectile(int x, int y, bool fromPlayer);
        std::optional<Entity> getEntityFromPlayerId(int playerId) const;
        std::size_t getPlayerCount() const;
        Entity addPlayer(int id);
        void removePlayer(Entity player);
        void applyShot(Entity shot, Entity target);

    public:
        Game(float tickRate);
        ~Game() = default;

        Registry &getRegistry() { return _engine.getRegistry(); }
        const std::vector<Entity> &getEntities() const { return _engine.getEntities(); }
        unsigned int getScore() const { return _score; }
        unsigned int getTick() const { return _tick; }

        std::vector<rtype::GameOutput> takeOutputQueue();

        void addInputQueue(const std::vector<rtype::GameInput> &input);
        void init();
        void update();
};
}

#endif
