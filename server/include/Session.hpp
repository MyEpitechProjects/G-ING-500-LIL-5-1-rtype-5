#ifndef SESSION_HPP
    #define SESSION_HPP

    #include <asio.hpp>
    #include <chrono>

    struct Session {
        std::size_t client_id;
        std::chrono::steady_clock::time_point lastSeen = std::chrono::steady_clock::now();
    };
    

#endif