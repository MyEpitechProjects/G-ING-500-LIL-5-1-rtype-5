#ifndef NETWORK_HANDLER_HPP
    #define NETWORK_HANDLER_HPP


    #include <asio.hpp>
    #include <iostream>
    #include <array>
    #include <queue>
    #include <memory>
    #include <variant>

    #include "WakeSignal.hpp"
    #include "Paquet.hpp"

    using asio::ip::udp;

namespace Net {

    template<typename Packet>
    struct OutgoingPacket {
        udp::endpoint destination;
        Packet packet;
    };

    using ToClientPacket = std::variant<SnapshotPacket, WelcomePacket>;
    using ToServerPacket = std::variant<InputPacket, HelloPacket>;

    class NetworkHandler {
        private:
            asio::io_context _io;
            udp::socket _socket;

            std::array<std::uint8_t, 65536> _receivedQueue;
            udp::endpoint _sender;

            std::shared_ptr<WakeSignal> _wakeSignal;
            std::queue<OutgoingPacket<ToClientPacket>> _clientQueue;
            std::queue<OutgoingPacket<ToServerPacket>> _serverQueue;

            std::mutex _outgoingMutex;

            void flushOutgoing();

        public:
            NetworkHandler(int port, std::shared_ptr<WakeSignal> wake);
            ~NetworkHandler();

            void receive();

            void run();

            void stop();

            void send(ToClientPacket packet, udp::endpoint destination);
            void send(ToServerPacket packet, udp::endpoint destination);
    };
}

#endif