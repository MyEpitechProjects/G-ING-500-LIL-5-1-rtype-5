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
        SparseArray<Component>& register_component();

        template <class Component>
        SparseArray<Component>& get_components();

        template <class Component>
        SparseArray<Component> const & get_components() const;

        void erase_entity(Entity const &entity);

    protected:
    private:
        std::unordered_map<std::type_index, SparseArray<std::any>> _components_array;

        std::vector<std::function<void(Registry &, Entity const &)>> _erasers;
};

#endif /* !REGISTRY_HPP */
