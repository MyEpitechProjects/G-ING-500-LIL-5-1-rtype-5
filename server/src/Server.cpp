#include "Server.hpp"

rtype::Server::Server(int port) : _wakeSignal(std::move(std::make_shared<WakeSignal>())), _netHandler(port, _wakeSignal), _idCounter(0) {

}

rtype::Server::~Server() {
    _netHandler.stop();
    if (_netThread.joinable())
        _netThread.join();
}

void rtype::Server::run() {
    _netThread = std::thread([this]{
        try {
            _netHandler.run();
        } catch (const std::exception& error) {
            std::cerr
                << "Erreur du thread reseau : "
                << error.what() << '\n';
            _netHandler.stop();
        }
    });

    while (_running) {
        _wakeSignal->wait();
        std::cout << "packet received" << std::endl;
        auto packet = _netHandler.popPacket();
        if (packet.has_value()) {
            handlePacket(packet.value());
        }
    }
}

void rtype::Server::handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet> &incomingPacket) {
    std::visit([](auto&& arg){
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Net::HelloPacket>)
            std::cout << "Send welcome packet" << std::endl;

    }, incomingPacket.packet);
}