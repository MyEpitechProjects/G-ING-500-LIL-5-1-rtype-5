/*
** EPITECH PROJECT, 2026
** PhysicsModule
** File description:
** PhysicsModule def
*/

#ifndef PHYSICSMODULE
    #define PHYSICSMODULE

    #include "AModule.hpp"
    #include "Entity.hpp"
    #include "Registry.hpp"
    #include <functional>
    #include <utility>
    #include <vector>

class PhysicsModule : public AModule {
    private:
        int _left;
        int _top;
        int _right;
        int _bottom;
        float _gravity;
        std::function<void(Entity)> _onOutOfBounds;
        std::vector<std::function<void(Registry &, Entity, Entity)>> _collisionHandlers;

        void handleCollisions(Registry &registry);

    public:
        PhysicsModule(int left, int top, int right, int bottom, float gravity, std::function<void(Entity)> onOutOfBounds);
        ~PhysicsModule() override = default;

        float getGravity() const { return _gravity; }
        void setGravity(float gravity) { _gravity = gravity; }

        template <class First, class Second>
        void onCollision(std::function<void(Registry &, Entity, Entity)> callback) {
            _collisionHandlers.push_back([callback](Registry &registry, Entity left, Entity right) {
                if (registry.hasComponent<First>(left) && registry.hasComponent<Second>(right))
                    callback(registry, left, right);
                else if (registry.hasComponent<First>(right) && registry.hasComponent<Second>(left))
                    callback(registry, right, left);
            });
        }

        void registerComponents(Registry &registry) override;
        void update(Registry &registry, double elapsedSeconds) override;
};

#endif
