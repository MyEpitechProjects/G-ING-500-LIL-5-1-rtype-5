/*
** EPITECH PROJECT, 2026
** Engine
** File description:
** Engine def
*/

#ifndef ENGINE
    #define ENGINE

    #include "EngineSystems.hpp"
    #include "Entity.hpp"
    #include "Hitbox.hpp"
    #include "IModule.hpp"
    #include "Position.hpp"
    #include "Registry.hpp"
    #include "SparseArray.hpp"
    #include "Velocity.hpp"
    #include "Zipper.hpp"
    #include <memory>
    #include <utility>
    #include <vector>

class Engine {
    private:
        Registry _registry;
        std::vector<std::unique_ptr<IModule>> _modules;

    public:
        Engine() = default;
        ~Engine() = default;

        Registry &getRegistry() { return _registry; }
        const Registry &getRegistry() const { return _registry; }

        template <class Component>
        SparseArray<Component> &registerComponent() { return _registry.registerComponent<Component>(); }

        template <class Component>
        SparseArray<Component> &getComponents() { return _registry.getComponents<Component>(); }

        template <class Component>
        SparseArray<Component> const &getComponents() const { return _registry.getComponents<Component>(); }

        template <class Component>
        typename SparseArray<Component>::reference_type addComponent(Entity const &entity, Component component) {
            return _registry.addComponent<Component>(entity, std::move(component));
        }

        template <class Component>
        void removeComponent(Entity const &entity) { _registry.removeComponent<Component>(entity); }

        template <class Component>
        bool hasComponent(Entity const &entity) const { return _registry.hasComponent<Component>(entity); }

        Entity spawnEntity() { return _registry.spawnEntity(); }
        void killEntity(Entity const &entity) { _registry.killEntity(entity); }
        bool isAlive(Entity const &entity) const { return _registry.isAlive(entity); }
        const std::vector<Entity> &getEntities() const { return _registry.getEntities(); }

        void addModule(std::unique_ptr<IModule> module) {
            module->registerComponents(_registry);
            _modules.push_back(std::move(module));
        }

        void update(double elapsedSeconds) {
            for (const std::unique_ptr<IModule> &module : _modules)
                module->update(_registry, elapsedSeconds);
        }

        template <class System, class... Args>
        void applySystem(System &&system, Args &&...args) {
            system(_registry, args...);
        }
};

#endif
