#ifndef NETWORK_HANDLER_HPP
    #define NETWORK_HANDLER_HPP


    #include <asio.hpp>
    #include <iostream>
    #include <array>
    #include <thread>

    using asio::ip::udp;

namespace Net {
    class NetworkHandler {
        private:
            asio::io_context _io;
            udp::socket _socket;

            std::array<std::uint8_t, 65536> _receivedQueue;
            udp::endpoint _sender;

            std::thread _networkThread;
        public:
            NetworkHandler(int port);
            ~NetworkHandler();

            void receive();

            void run();

            void stop();
    };
}

#endif