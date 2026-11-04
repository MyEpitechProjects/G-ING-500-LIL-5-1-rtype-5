#ifndef INSTANCE_HPP
    #define INSTANCE_HPP

    #include "Game.hpp"
    #include "WakeSignal.hpp"

    #include <atomic>
    #include <cstddef>
    #include <cstdint>
    #include <memory>
    #include <mutex>
    #include <vector>

class Instance {
    private:
        std::uint32_t _id;
        std::shared_ptr<WakeSignal> _wakeSignal;
        rtype::Game _game;
        std::atomic<bool> _updating = false;
        std::mutex _queuesMutex;
        std::vector<rtype::GameInput> _pendingInputs;
        std::vector<rtype::GameOutput> _pendingOutputs;

        void queueInput(const rtype::GameInput &input);

    public:
        Instance(std::shared_ptr<WakeSignal> wakeSignal, std::uint32_t id);
        ~Instance() = default;

        std::uint32_t getId() const;

        void addPlayer(std::size_t clientId);
        void removePlayer(std::size_t clientId);
        void setControls(std::size_t clientId, const Controllable &controls);

        std::vector<rtype::GameOutput> takeOutputs();

        bool tryStartUpdate();
        void update();
};

#endif
