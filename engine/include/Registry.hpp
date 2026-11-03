/*
** EPITECH PROJECT, 2026
** Registry
** File description:
** Registry def
*/

#ifndef REGISTRY
    #define REGISTRY

    #include "Entity.hpp"
    #include "SparseArray.hpp"
    #include <algorithm>
    #include <any>
    #include <exception>
    #include <functional>
    #include <string>
    #include <typeindex>
    #include <unordered_map>
    #include <utility>
    #include <vector>

class Registry {
    private:
        std::unordered_map<std::type_index, std::any> _componentArrays;
        std::size_t _nextId;
        std::vector<Entity> _entities;
        std::vector<std::function<void(Registry &, Entity const &)>> _erasers;

    public:
        Registry() : _nextId(0) {}
        ~Registry() = default;

        template <class Component>
        SparseArray<Component> &registerComponent() {
            if (_componentArrays.find(std::type_index(typeid(Component))) != _componentArrays.end())
                return getComponents<Component>();
            _componentArrays.emplace(std::type_index(typeid(Component)), SparseArray<Component>{});
            _erasers.push_back([](Registry &registry, Entity const &entity) {
                registry.getComponents<Component>().erase(entity.getId());
            });
            return getComponents<Component>();
        }

        template <class Component>
        SparseArray<Component> &getComponents() {
            std::unordered_map<std::type_index, std::any>::iterator found = _componentArrays.find(std::type_index(typeid(Component)));

            if (found == _componentArrays.end())
                throw Exception("Registry: component type is not registered");
            return std::any_cast<SparseArray<Component> &>(found->second);
        }

        template <class Component>
        SparseArray<Component> const &getComponents() const {
            std::unordered_map<std::type_index, std::any>::const_iterator found = _componentArrays.find(std::type_index(typeid(Component)));

            if (found == _componentArrays.end())
                throw Exception("Registry: component type is not registered");
            return std::any_cast<SparseArray<Component> const &>(found->second);
        }

        template <class Component>
        typename SparseArray<Component>::reference_type addComponent(Entity const &entity, Component component) {
            return getComponents<Component>().insertAt(entity.getId(), std::move(component));
        }

        template <class Component>
        void removeComponent(Entity const &entity) {
            getComponents<Component>().erase(entity.getId());
        }

        template <class Component>
        bool hasComponent(Entity const &entity) const {
            SparseArray<Component> const &components = getComponents<Component>();

            return entity.getId() < components.size() && components[entity.getId()].has_value();
        }

        void eraseEntity(Entity const &entity) {
            for (const std::function<void(Registry &, Entity const &)> &eraser : _erasers)
                eraser(*this, entity);
        }

        Entity spawnEntity() {
            Entity entity(_nextId++);

            _entities.push_back(entity);
            return entity;
        }

        void killEntity(Entity const &entity) {
            std::vector<Entity>::iterator found = std::find_if(_entities.begin(), _entities.end(),
                [&entity](const Entity &current) {
                    return current.getId() == entity.getId();
                }
            );

            if (found == _entities.end())
                return;
            eraseEntity(entity);
            _entities.erase(found);
        }

        bool isAlive(Entity const &entity) const {
            return std::any_of(_entities.begin(), _entities.end(),
                [&entity](const Entity &current) {
                    return current.getId() == entity.getId();
                }
            );
        }

        const std::vector<Entity> &getEntities() const { return _entities; }

        class Exception : public std::exception {
            private:
                std::string _message;
            public:
                Exception(const std::string &message) : _message(message) {}
                const char *what() const noexcept override { return _message.c_str(); }
        };
};

#endif
