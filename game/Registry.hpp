/*
** EPITECH PROJECT, 2026
** Registry.hpp
** File description:
** Registry class definition
*/

#ifndef REGISTRY_HPP
    #define REGISTRY_HPP

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "SparseArray.hpp"
#include "Entity.hpp"

class Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        template <class Component>
        SparseArray<Component> &register_component() {
            const std::type_index type(typeid(Component));
            if (!_components_array.contains(type)) {
                _components_array.emplace(type, SparseArray<Component>{});
                _erasers.push_back([](Registry &registry, const Entity &entity) {
                    registry.get_components<Component>().erase(entity.id());
                });
            }
            return std::any_cast<SparseArray<Component> &>(_components_array.at(type));
        }

        template <class Component>
        SparseArray<Component> &get_components() {
            return std::any_cast<SparseArray<Component> &>(
                _components_array.at(std::type_index(typeid(Component))));
        }

        template <class Component>
        const SparseArray<Component> &get_components() const {
            return std::any_cast<const SparseArray<Component> &>(
                _components_array.at(std::type_index(typeid(Component))));
        }

        void erase_entity(const Entity &entity) {
            for (const std::function<void(Registry &, const Entity &)> &eraser : _erasers)
                eraser(*this, entity);
        }

    private:
        std::unordered_map<std::type_index, std::any> _components_array;
        std::vector<std::function<void(Registry &, const Entity &)>> _erasers;
};

#endif /* !REGISTRY_HPP */
