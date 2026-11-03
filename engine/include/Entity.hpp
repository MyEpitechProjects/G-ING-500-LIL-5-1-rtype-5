/*
** EPITECH PROJECT, 2026
** Entity
** File description:
** Entity def
*/

#ifndef ENTITY
    #define ENTITY

    #include <cstddef>

class Entity {
    private:
        std::size_t _id;

    public:
        explicit Entity(std::size_t id) : _id(id) {}
        ~Entity() = default;

        std::size_t getId() const { return _id; }
};

#endif
