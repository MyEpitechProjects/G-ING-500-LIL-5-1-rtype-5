/*
** EPITECH PROJECT, 2026
** AnimationSystem
** File description:
** AnimationSystem
*/

#include "RenderSystems.hpp"
#include "Zipper.hpp"

#include <tuple>

void AnimationSystem::operator()(Registry &registry, double elapsedSeconds) const {
    SparseArray<SpriteAnimation> &animations = registry.getComponents<SpriteAnimation>();

    for (std::tuple<SpriteAnimation &> components : Containers::Zipper(animations))
        std::get<0>(components).time += elapsedSeconds;
}
