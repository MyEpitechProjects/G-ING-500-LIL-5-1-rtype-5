#ifndef GAME_MESSAGE_HPP
    #define GAME_MESSAGE_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Components.hpp"

namespace rtype {
    enum class GameInputType {
        AddPlayer,
        RemovePlayer,
        SetControls,
    };

    struct GameInput {
        GameInputType type;
        std::size_t playerId = 0;
        Controllable controls{};
    };

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
