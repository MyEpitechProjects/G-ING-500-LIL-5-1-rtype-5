/*
** EPITECH PROJECT, 2026
** CooldownSystem
** File description:
** CooldownSystem
*/

#include "GameSystems.hpp"
#include "CoolDown.hpp"
#include "Zipper.hpp"

#include <algorithm>
#include <tuple>

void rtype::CooldownSystem::operator()(Registry &registry, double elapsedSeconds) const {
    SparseArray<CoolDown> &cooldowns = registry.getComponents<CoolDown>();

    for (std::tuple<CoolDown &> components : Containers::Zipper(cooldowns)) {
        CoolDown &cooldown = std::get<0>(components);

        if (cooldown.shootCooldown > 0)
            cooldown.shootCooldown = std::max(0.0, cooldown.shootCooldown - elapsedSeconds);
    }
}
