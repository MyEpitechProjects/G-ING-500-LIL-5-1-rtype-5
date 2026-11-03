/*
** EPITECH PROJECT, 2026
** ControlAssignmentSystem
** File description:
** ControlAssignmentSystem
*/

#include "GameSystems.hpp"

void rtype::ControlAssignmentSystem::operator()(Registry &registry, Entity player, const Controllable &controls) const {
    registry.addComponent(player, controls);
}
