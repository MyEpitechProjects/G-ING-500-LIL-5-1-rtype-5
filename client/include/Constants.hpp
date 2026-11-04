#pragma once
#include "Paquet.hpp"

namespace rtype::config {
    constexpr unsigned WINDOW_WIDTH = 800;
    constexpr unsigned WINDOW_HEIGHT = 600;

    constexpr double SERVER_TICK_RATE = static_cast<double>(Net::SERVER_TICK_RATE);
    constexpr double INTERP_DELAY = 2.0 / SERVER_TICK_RATE;
    constexpr double MAX_EXTRAPOLATION = 0.1;

    // used for predictions (don't affect server)
    constexpr float PLAYER_SPEED = 5.0f * static_cast<float>(SERVER_TICK_RATE);   // px/s
    constexpr float FIELD_WIDTH = 800.0f;
    constexpr float FIELD_HEIGHT = 600.0f;
    constexpr float SNAP_DISTANCE = 120.0f;
    constexpr float IDLE_BEFORE_RECONCILE = 0.4f;
    constexpr float RECONCILE_RATE = 10.0f;
}
