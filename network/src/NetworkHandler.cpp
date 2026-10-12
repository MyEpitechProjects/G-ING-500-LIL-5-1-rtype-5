#include "NetworkHandler.hpp"
#include "PacketCodec.hpp"
#include <utility>

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
            auto packet = PacketCodec::deserialize(_receivedQueue.data(), received);
            if (packet) {
                {
                    std::lock_guard<std::mutex> lock(_incomingMutex);
                    _incomingQueue.push({_sender, std::move(packet)});
                }
                if (_wakeSignal)
                    _wakeSignal->notify();
            } else {
                std::cout << "invalid packet" << std::endl;
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

void Net::NetworkHandler::send(Net::PacketCodec::Packet packet, udp::endpoint destination) {
    {
        std::lock_guard<std::mutex> lock(_outgoingMutex);
        _outgoingQueue.push({std::move(destination), std::move(packet)});
    }

    asio::post(_io, [this] {
        flushOutgoing();
    });
}

std::optional<Net::IncomingPacket<Net::PacketCodec::Packet>> Net::NetworkHandler::popPacket() {
    std::lock_guard<std::mutex> lock(_incomingMutex);
    if (_incomingQueue.empty())
        return std::nullopt;
    auto packet = std::move(_incomingQueue.front());
    _incomingQueue.pop();
    return packet;
}

void Net::NetworkHandler::flushOutgoing() {
    std::lock_guard<std::mutex> lock(_outgoingMutex);
    while (!_outgoingQueue.empty()) {
        OutgoingPacket<Net::PacketCodec::Packet> packet = std::move(_outgoingQueue.front());
        _outgoingQueue.pop();
        auto bytes = packet.packet->serialize();
        _socket.async_send_to(asio::buffer(bytes), packet.destination, [bytes](const asio::error_code& error, [[maybe_unused]]std::size_t sent) {
            if (error) {
                std::cerr << "Erreur d'envoi : " << error.message() << '\n';
            }
        });
    }
}
