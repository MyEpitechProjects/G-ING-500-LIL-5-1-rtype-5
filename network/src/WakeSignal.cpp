#include "WakeSignal.hpp"

void WakeSignal::notify() {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _pending = true;
    }
    _condition.notify_one();
}

void WakeSignal::wait() {
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return _pending; });
    _pending = false;
}