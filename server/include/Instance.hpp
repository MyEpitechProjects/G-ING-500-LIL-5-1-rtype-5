#ifndef INSTANCE_HPP
    #define INSTANCE_HPP

    #include "Game.hpp"
    #include "WakeSignal.hpp"

    #include <cstddef>
    #include <cstdint>
    #include <memory>

class Instance {
    private:
        std::uint32_t _id;
        rtype::Game _game;

    public:
        Instance(std::shared_ptr<WakeSignal> wakeSignal, std::uint32_t id);
        ~Instance() = default;

        std::uint32_t getId() const;

        void addPlayer(std::size_t clientId);
        void removePlayer(std::size_t clientId);
};

#endif
