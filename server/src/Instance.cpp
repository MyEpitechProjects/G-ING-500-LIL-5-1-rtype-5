#include "Instance.hpp"

#include <iterator>

Instance::Instance(std::uint32_t id) : _id(id), _game() {
}

std::uint32_t Instance::getId() const {
    return _id;
}

void Instance::queueInput(const rtype::GameInput &input) {
    std::lock_guard<std::mutex> lock(_queuesMutex);
    _pendingInputs.push_back(input);
}

void Instance::addPlayer(std::size_t clientId) {
    queueInput(rtype::GameInput{rtype::GameInputType::AddPlayer, clientId});
}

void Instance::removePlayer(std::size_t clientId) {
    queueInput(rtype::GameInput{rtype::GameInputType::RemovePlayer, clientId});
}

void Instance::setControls(std::size_t clientId, const Controllable &controls) {
    queueInput(rtype::GameInput{rtype::GameInputType::SetControls, clientId, controls});
}

std::vector<rtype::GameOutput> Instance::takeOutputs() {
    std::vector<rtype::GameOutput> outputs;
    std::lock_guard<std::mutex> lock(_queuesMutex);
    outputs.swap(_pendingOutputs);
    return outputs;
}

bool Instance::tryStartUpdate() {
    return !_updating.exchange(true);
}

void Instance::update() {
    std::vector<rtype::GameInput> inputs;
    {
        std::lock_guard<std::mutex> lock(_queuesMutex);
        inputs.swap(_pendingInputs);
    }
    _game.addInputQueue(inputs);
    _game.update();
    std::vector<rtype::GameOutput> outputs = _game.takeOutputQueue();
    {
        std::lock_guard<std::mutex> lock(_queuesMutex);
        _pendingOutputs.insert(_pendingOutputs.end(), std::make_move_iterator(outputs.begin()), std::make_move_iterator(outputs.end()));
    }
    _updating = false;
    if (_wakeSignal)
        _wakeSignal->notify();
}
