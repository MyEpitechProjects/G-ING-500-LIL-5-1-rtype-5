#include "Instance.hpp"

Instance::Instance(std::shared_ptr<WakeSignal> wakeSignal, std::uint32_t id) : _id(id), _wakeSignal(std::move(wakeSignal)) {
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

void Instance::setControls(std::size_t clientId, const Controllable &controls) {
    _game.addInputQueue({rtype::GameInput{rtype::GameInputType::SetControls, clientId, controls}});
}

std::vector<rtype::GameOutput> Instance::takeOutputs() {
    return _game.takeOutputQueue();
}

bool Instance::tryStartUpdate() {
    return !_updating.exchange(true);
}

void Instance::update() {
    _game.update();
    _updating = false;
    if (_wakeSignal)
        _wakeSignal->notify();
}
