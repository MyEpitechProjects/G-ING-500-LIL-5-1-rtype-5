#include "NetworkHandler.hpp"

Net::NetworkHandler::NetworkHandler(int port) : _socket(_io,udp::endpoint(udp::v4(), port)) {

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
                // Ici : valider et décoder le paquet.
                // Ne lire que receiveBuffer_[0..received).
            }
            receive();
        }
    );
}

void Net::NetworkHandler::run() {
    receive();
    _networkThread = std::thread([this]{
        try {
            _io.run();
        } catch (const std::exception& error) {
            std::cerr
                << "Erreur du thread reseau : "
                << error.what() << '\n';
            _io.stop();
        }
    });
}

void Net::NetworkHandler::stop() {
    _io.stop();

    if (_networkThread.joinable())
        _networkThread.join();
}

bool Net::NetworkHandler::queueIsEmpty() {
    return _receivedQueue.empty();
}