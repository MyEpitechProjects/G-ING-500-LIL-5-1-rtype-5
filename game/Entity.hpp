#ifndef ENTITY_HPP
    #define ENTITY_HPP

    #include <cstddef>

class Entity {
    private:
        std::size_t _id;
    public:
        explicit Entity(std::size_t id) : _id(id) {}
        ~Entity() = default;

        std::size_t id() const {
            return _id;
        }
};

#endif
