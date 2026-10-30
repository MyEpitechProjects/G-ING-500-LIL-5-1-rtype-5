/*
** EPITECH PROJECT, 2026
** GameOutput
** File description:
** GameOutput def
*/

#ifndef GAMEOUTPUT
    #define GAMEOUTPUT

    #include "Position.hpp"
    #include <cstdint>
    #include <vector>

namespace rtype {
enum class GameEntityKind {
    Player,
    Enemy,
    Projectile,
};

struct GameEntityState {
    std::size_t id;
    GameEntityKind kind;
    Position position;
    int health = 0;
    bool fromPlayer = false;
};

enum class GameOutputType {
    PlayerCreated,
    EntityDestroyed,
    Snapshot,
};

struct GameOutput {
    GameOutputType type;
    std::uint32_t clientId = 0;
    std::size_t entityId = 0;
    unsigned int tick = 0;
    unsigned int score = 0;
    std::vector<GameEntityState> entities;
};
}

#endif
