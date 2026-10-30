/*
** EPITECH PROJECT, 2026
** Game
** File description:
** Game
*/

#include "Game.hpp"
#include "Controllable.hpp"
#include "CoolDown.hpp"
#include "Enemy.hpp"
#include "GameSystems.hpp"
#include "Health.hpp"
#include "Player.hpp"
#include "Projectile.hpp"

#include <cstdint>

rtype::Game::Game() : _width(800), _height(600), _playerSpeed(5), _shotInterval(0.3), _enemyInterval(60), _tick(0), _score(0), _lastUpdate(std::chrono::steady_clock::now()) {
    init();
}

void rtype::Game::destroyEntity(Entity entity) {
    rtype::GameOutput output{};

    if (!_engine.isAlive(entity))
        return;
    output.type = rtype::GameOutputType::EntityDestroyed;
    output.entityId = entity.getId();
    output.score = _score;
    _out.push_back(output);
    _engine.killEntity(entity);
}

Entity rtype::Game::spawnEnemy(int y) {
    Entity enemy = _engine.spawnEntity();

    rtype::EnemySpawnSystem{}(_engine.getRegistry(), enemy, _width - 20, y);
    return enemy;
}

Entity rtype::Game::spawnProjectile(int x, int y, bool fromPlayer) {
    Entity projectile = _engine.spawnEntity();

    rtype::ProjectileSpawnSystem{}(_engine.getRegistry(), projectile, x, y, fromPlayer);
    return projectile;
}

std::optional<Entity> rtype::Game::getEntityFromPlayerId(int playerId) const {
    const SparseArray<Player> &players = _engine.getComponents<Player>();

    for (const Entity &entity : _engine.getEntities())
        if (_engine.hasComponent<Player>(entity) && players[entity.getId()]->id == playerId)
            return entity;
    return std::nullopt;
}

std::size_t rtype::Game::getPlayerCount() const {
    std::size_t count = 0;

    for (const Entity &entity : _engine.getEntities())
        if (_engine.hasComponent<Player>(entity))
            ++count;
    return count;
}

Entity rtype::Game::addPlayer(int id) {
    Entity player = _engine.spawnEntity();
    rtype::GameOutput output{};

    rtype::PlayerSpawnSystem{}(_engine.getRegistry(), player, id);
    output.type = rtype::GameOutputType::PlayerCreated;
    output.clientId = static_cast<std::uint32_t>(id);
    output.entityId = player.getId();
    output.score = _score;
    _out.push_back(output);
    return player;
}

void rtype::Game::removePlayer(Entity player) {
    rtype::PlayerRemovalSystem{}(_engine.getRegistry(), player,
        [this](Entity entity) { destroyEntity(entity); }
    );
}

std::vector<rtype::GameOutput> rtype::Game::takeOutputQueue() {
    std::vector<rtype::GameOutput> output;

    output.swap(_out);
    return output;
}

void rtype::Game::addInputQueue(const std::vector<rtype::GameInput> &input) {
    _in.insert(_in.end(), input.begin(), input.end());
}

void rtype::Game::init() {
    _engine.registerComponent<Position>();
    _engine.registerComponent<Velocity>();
    _engine.registerComponent<Hitbox>();
    _engine.registerComponent<Health>();
    _engine.registerComponent<Controllable>();
    _engine.registerComponent<Player>();
    _engine.registerComponent<Enemy>();
    _engine.registerComponent<Projectile>();
    _engine.registerComponent<CoolDown>();
}

void rtype::Game::update() {
    std::chrono::steady_clock::time_point now;
    double elapsedSeconds = 0;

    _tick++;
    rtype::InputQueueSystem{}(_in, _engine.getRegistry(),
        [this](int playerId) { return getEntityFromPlayerId(playerId); },
        [this](int playerId) { addPlayer(playerId); },
        [this](Entity player) { removePlayer(player); }
    );
    now = std::chrono::steady_clock::now();
    elapsedSeconds = std::chrono::duration<double>(now - _lastUpdate).count();
    _lastUpdate = now;

    rtype::CooldownSystem{}(_engine.getRegistry(), elapsedSeconds);
    rtype::ControlSystem{}(_engine.getRegistry(), _playerSpeed);
    rtype::ShotSystem{}(_engine.getRegistry(), _shotInterval,
        [this](int x, int y, bool fromPlayer) { spawnProjectile(x, y, fromPlayer); }
    );
    rtype::EnemySystem{}(_engine.getRegistry(), _tick, _enemyInterval, _height,
        [this](int y) { spawnEnemy(y); },
        [this](int x, int y, bool fromPlayer) { spawnProjectile(x, y, fromPlayer); }
    );
    PositionSystem{}(_engine.getRegistry());
    rtype::BoundsSystem{}(_engine.getRegistry(), _width, _height);
    OutOfBoundsSystem{}(_engine.getRegistry(), -20, -20, _width + 20, _height + 20,
        [this](Entity entity) { destroyEntity(entity); }
    );
    rtype::ProjectileCollisionSystem{}(_engine.getRegistry(), _score,
        [this](Entity entity) { destroyEntity(entity); }
    );
    rtype::EnemyCollisionSystem{}(_engine.getRegistry(),
        [this](Entity entity) { destroyEntity(entity); }
    );
    rtype::SnapshotSystem{}(_engine.getRegistry(), _engine.getEntities(), _tick, _score, _out);
}
