/*
** EPITECH PROJECT, 2026
** Registry.cpp
** File description:
** Registry class implementation
*/
#include "Registry.hpp"

template <class Component>
SparseArray<Component> &Registry::register_component() {
    _components_array.insert(std::make_pair<std::type_index, SparseArray<Component>>(typeid(Component), SparseArray<Component>{}));

    _erasers.push_back([](Registry &registre, Entity const &entity){
        registre.get_components<Component>().erase(entity);
    })
    return std::any_cast<SparseArray<Component> &>(_components_array.at(std::type_index(typeid(Component))));
}

template <class Component>
SparseArray<Component> &Registry::get_components() {
    return std::any_cast<SparseArray<Component> &>(_components_array.at(std::type_index(typeid(Component))));
}

template <class Component>
SparseArray<Component> const &Registry::get_components() const {
    return std::any_cast<SparseArray<Component> const &>(_components_array.at(std::type_index(typeid(Component))));
}

void Registry::erase_entity(Entity const &entity) {
    for (const std::function<void(Registry &, Entity const &)> &eraser : _erasers)
        eraser(*this, entity);
}
