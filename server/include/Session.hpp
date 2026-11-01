#ifndef SESSION_HPP
    #define SESSION_HPP

    #include "IPacket.hpp"

    #include <asio.hpp>
    #include <chrono>
    #include <cstddef>
    #include <cstdint>
    #include <memory>
    #include <unordered_map>

    struct PendingEvent {
        std::shared_ptr<IPacket> packet;
        std::chrono::steady_clock::time_point nextRetry;
        unsigned int attempts = 0;
    };

    struct Session {
        std::size_t client_id = 0;
        std::size_t instance_id = 0;
        std::unordered_map<std::uint32_t, PendingEvent> pendingEvents;
        std::chrono::steady_clock::time_point lastSeen = std::chrono::steady_clock::now();
    };

#endif
