#include "Server.hpp"

rtype::Server::Server(int port) : _wakeSignal(std::move(std::make_shared<WakeSignal>())), _netHandler(port, _wakeSignal), _game(_wakeSignal), _idCounter(0) {

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
            std::cerr << "Erreur du thread reseau : " << error.what() << std::endl;
            _netHandler.stop();
        }
    });

    _gameThread = std::thread([this]{
        try {
            _game.run();
        } catch (const std::exception& error) {
            std::cerr << "Erreur du thread jeu : " << error.what() << std::endl;
            _game.stop();
        }
    });

    while (_running) {
        _wakeSignal->wait();
        while (auto packet = _netHandler.popPacket()) {
            std::cout << "network packet received" << std::endl;
            handlePacket(packet.value());
        }
    }
}

void rtype::Server::handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet> &incomingPacket) {
    if (dynamic_cast<Net::HelloPacket*>(incomingPacket.packet.get())) {
        Session playerSession = createSession(incomingPacket.sender);
        Net::WelcomePacket weclome;
        weclome.playerEntityId = playerSession.client_id;
        std::unique_ptr<Net::WelcomePacket> packet = std::make_unique<Net::WelcomePacket>(weclome);
        _netHandler.send(std::move(packet), incomingPacket.sender);
    }
    if (dynamic_cast<Net::InputPacket*>(incomingPacket.packet.get()))
        std::cout << "send this packet to game logic" << std::endl;
}

Session rtype::Server::createSession(asio::ip::udp::endpoint endpoint) {
    if (!_sessions.contains(endpoint)) {
        _sessions.insert({endpoint, Session{_idCounter}});
        _idCounter++;
    }

    return _sessions.at(endpoint);
}