/*
** EPITECH PROJECT, 2026
** InputQueueSystem
** File description:
** InputQueueSystem
*/

#include "GameSystems.hpp"

#include <limits>

void rtype::InputQueueSystem::operator()(Registry &registry, std::vector<rtype::GameInput> &queuedInput, const std::function<std::optional<Entity>(int)> &findPlayer, const std::function<void(int)> &addPlayer, const std::function<void(Entity)> &removePlayer) const {
    std::vector<rtype::GameInput> input;

    input.swap(queuedInput);
    for (const rtype::GameInput &message : input) {
        if (message.playerId > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            continue;
        const int playerId = static_cast<int>(message.playerId);
        const std::optional<Entity> player = findPlayer(playerId);

        if (message.type == rtype::GameInputType::AddPlayer) {
            if (!player)
                addPlayer(playerId);
        } else if (message.type == rtype::GameInputType::RemovePlayer) {
            if (player)
                removePlayer(*player);
        } else if (message.type == rtype::GameInputType::SetControls && player) {
            rtype::ControlAssignmentSystem{}(registry, *player, message.controls);
        }
    }
}
