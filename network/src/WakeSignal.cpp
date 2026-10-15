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

bool WakeSignal::waitFor(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    bool notified = _condition.wait_for(lock, timeout, [this] { return _pending; });

    if (notified) {
        _pending = false;
    }

    return notified;
}