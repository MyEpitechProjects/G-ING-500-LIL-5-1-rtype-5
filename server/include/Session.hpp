#ifndef SESSION_HPP
    #define SESSION_HPP

    #include <asio.hpp>
    #include <chrono>
    #include <unordered_map>

    struct PendingEvent {
        std::shared_ptr<IPacket> packet;
        std::chrono::steady_clock::time_point nextRetry;
        unsigned int attempts = 0;
    };

    struct Session {
        std::size_t client_id;
        std::unordered_map<std::uint32_t, PendingEvent> pendingEvents;
        std::chrono::steady_clock::time_point lastSeen = std::chrono::steady_clock::now();
    };
    

#endif
