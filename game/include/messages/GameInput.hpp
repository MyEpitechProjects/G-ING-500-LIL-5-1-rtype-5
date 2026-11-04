/*
** EPITECH PROJECT, 2026
** GameInput
** File description:
** GameInput def
*/

#ifndef GAMEINPUT
    #define GAMEINPUT

    #include "Controllable.hpp"

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
}

#endif
