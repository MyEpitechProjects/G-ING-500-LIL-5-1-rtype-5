#include "NetworkHandler.hpp"

Net::NetworkHandler::NetworkHandler(int port, std::shared_ptr<WakeSignal> wake): _socket(_io,udp::endpoint(udp::v4(), port)), _wakeSignal(std::move(wake)) {

}

Net::NetworkHandler::~NetworkHandler() {
    
}

void Net::NetworkHandler::receive() {
    _socket.async_receive_from(
        asio::buffer(_receivedQueue),
        _sender,
        [this]( const asio::error_code& error, std::size_t received) {
            if (error == asio::error::operation_aborted) {
                return;
            }
            if (error) {
                std::cerr << "Erreur de reception : " << error.message() << '\n';
                return;
            }
            if (received > 1200) {
                std::cerr << "Datagramme hors limite\n";
            } else {
                std::cout
                    << received << " octets recus de "
                    << _sender.address().to_string()
                    << ':' << _sender.port()
                    << '\n';
                _wakeSignal->notify();
            }
            receive();
        }
    );
}

void Net::NetworkHandler::run() {
    receive();
    _io.run();
}

void Net::NetworkHandler::stop() {
    _io.stop();
}

void Net::NetworkHandler::send(ToClientPacket packet, udp::endpoint destination) {
    {
        std::lock_guard<std::mutex> lock(_outgoingMutex);
        _clientQueue.push({std::move(destination), std::move(packet)});
    }

    asio::post(_io, [this] {
        flushOutgoing();
    });
}

void Net::NetworkHandler::send(
    ToServerPacket packet, udp::endpoint destination)
{
    {
        std::lock_guard<std::mutex> lock(_outgoingMutex);
        _serverQueue.push({std::move(destination), std::move(packet)});
    }

    asio::post(_io, [this] {
        flushOutgoing();
    });
}

void Net::NetworkHandler::flushOutgoing() {
    
}