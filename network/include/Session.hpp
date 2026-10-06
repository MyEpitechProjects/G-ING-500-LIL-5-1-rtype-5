#ifndef SESSION_HPP
    #define SESSION_HPP

    #include <asio.hpp>

struct Session {
    std::uint8_t client_id;
    asio::ip::udp::endpoint client_endpoint;
};

#endif