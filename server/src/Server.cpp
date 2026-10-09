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
        while (auto packet = _netHandler.popPacket()) {
            handlePacket(packet.value());
        }
    }
}

void rtype::Server::handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet> &incomingPacket) {
    if (dynamic_cast<Net::HelloPacket*>(incomingPacket.packet.get()))
        std::cout << "send welcome packet" << std::endl;
    if (dynamic_cast<Net::InputPacket*>(incomingPacket.packet.get()))
        std::cout << "send this packet to game logic" << std::endl;
}