#ifndef NETWORK_HANDLER_HPP
    #define NETWORK_HANDLER_HPP


    #include <asio.hpp>
    #include <iostream>
    #include <array>
    #include <queue>
    #include <memory>
    #include <variant>
    #include <mutex>
    #include <optional>
    #include <chrono>

    #include "WakeSignal.hpp"
    #include "Paquet.hpp"
    #include "PacketCodec.hpp"

namespace Net {

    using asio::ip::udp;

    template<typename Packet>
    struct OutgoingPacket {
        udp::endpoint destination;
        Packet packet;
    };

    template<typename Packet>
    struct IncomingPacket {
        udp::endpoint sender;
        Packet packet;
        std::chrono::steady_clock::time_point receivedAt{};
    };

    class NetworkHandler {
        private:
            asio::io_context _io;
            udp::socket _socket;

            std::array<std::uint8_t, 65536> _receivedQueue;
            udp::endpoint _sender;

            std::shared_ptr<WakeSignal> _wakeSignal;

            std::mutex _outgoingMutex;
            std::mutex _incomingMutex;

            std::queue<OutgoingPacket<Net::PacketCodec::Packet>> _outgoingQueue;
            std::queue<IncomingPacket<Net::PacketCodec::Packet>> _incomingQueue;

            void flushOutgoing();

        public:
            NetworkHandler(int port, std::shared_ptr<WakeSignal> wake);
            ~NetworkHandler();

            void receive();

            void run();

            void stop();

            void send(Net::PacketCodec::Packet packet, udp::endpoint destination);

            std::optional<IncomingPacket<Net::PacketCodec::Packet>> popPacket();
    };
}

#endif
