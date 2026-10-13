/*
** EPITECH PROJECT, 2026
** Registry.hpp
** File description:
** Registry class definition
*/

#ifndef REGISTRY_HPP
    #define REGISTRY_HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <typeindex>
#include <any>
#include "SparseArray.hpp"
#include "Entity.hpp"
	

class Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        template <class Component>
        SparseArray<Component>& register_component() {
            _components_array.insert(std::make_pair<std::type_index, SparseArray<Component>>(typeid(Component), SparseArray<Component>{}));
            _erasers.push_back([](Registry &registre, Entity const &entity){
                registre.get_components<Component>().erase(entity.id());
            });
            return std::any_cast<SparseArray<Component> &>(_components_array.at(std::type_index(typeid(Component))));
        }

        template <class Component>
        SparseArray<Component>& get_components() {
            return std::any_cast<SparseArray<Component> &>(_components_array.at(std::type_index(typeid(Component))));
        }

        template <class Component>
        SparseArray<Component> const & get_components() const {
            return std::any_cast<SparseArray<Component> const &>(_components_array.at(std::type_index(typeid(Component))));
        }

        void erase_entity(Entity const &entity) {
            for (const std::function<void(Registry &, Entity const &)> &eraser : _erasers)
                eraser(*this, entity);
        }

    protected:
    private:
        std::unordered_map<std::type_index, std::any> _components_array;

        std::vector<std::function<void(Registry &, Entity const &)>> _erasers;
};

#endif /* !REGISTRY_HPP */
