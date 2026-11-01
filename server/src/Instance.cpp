#include "Instance.hpp"

Instance::Instance(std::shared_ptr<WakeSignal> wakeSignal, std::uint32_t id) : _id(id), _game(wakeSignal) {
}

std::uint32_t Instance::getId() const {
    return _id;
}

void Instance::addPlayer(std::size_t clientId) {
    _game.addInputQueue({rtype::GameInput{rtype::GameInputType::AddPlayer, clientId}});
}

void Instance::removePlayer(std::size_t clientId) {
    _game.addInputQueue({rtype::GameInput{rtype::GameInputType::RemovePlayer, clientId}});
}
