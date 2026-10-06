#ifndef NETWORK_HANDLER_HPP
    #define NETWORK_HANDLER_HPP

    #include "Receiver.hpp"
    #include "Sender.hpp"

    using asio::ip::udp;

namespace Net {
    class NetworkHandler {
        private:
            Net::Receiver _receiver;
            Net::Sender _sender;

            asio::io_context _io;
            udp::socket _socket;
        public:
            NetworkHandler(int port);
            ~NetworkHandler();
    };

    NetworkHandler::NetworkHandler(int port) : _socket(_io,udp::endpoint(udp::v4(), port)) {

    }

    NetworkHandler::~NetworkHandler() {
    
    }
    
}

#endif