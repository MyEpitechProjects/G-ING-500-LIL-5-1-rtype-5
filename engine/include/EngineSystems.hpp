/*
** EPITECH PROJECT, 2026
** EngineSystems
** File description:
** EngineSystems def
*/

#ifndef ENGINESYSTEMS
    #define ENGINESYSTEMS

    #include "Entity.hpp"
    #include "Registry.hpp"
    #include <functional>

class OutOfBoundsSystem {
    public:
        void operator()(Registry &registry, int left, int top, int right, int bottom, const std::function<void(Entity)> &onOut) const;
};

class PositionSystem {
    public:
        void operator()(Registry &registry) const;
};

#endif
