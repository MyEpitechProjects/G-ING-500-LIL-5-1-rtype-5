#include "Server.hpp"

rtype::Server::Server(int port) : _wakeSignal(std::move(std::make_shared<WakeSignal>())), _netHandler(port, _wakeSignal), _game(_wakeSignal), _idCounter(0) {

}

rtype::Server::~Server() {
    _netHandler.stop();
    if (_netThread.joinable())
        _netThread.join();

    _game.stop();
    if (_gameThread.joinable())
        _gameThread.join();
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
        std::vector<GameOutput> snapshots = _game.takeOutputQueue();
        for (GameOutput snapshot : snapshots) {
            Net::SnapshotPacket snapPacket = convertGameOutput(snapshot);
            broadcast(snapPacket);
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
    if (auto packet = dynamic_cast<Net::InputPacket*>(incomingPacket.packet.get())) {
        Controllable inputs{(packet->buttons & Net::InputButton::UP) != 0, (packet->buttons & Net::InputButton::DOWN) != 0, (packet->buttons & Net::InputButton::RIGHT) != 0, (packet->buttons & Net::InputButton::LEFT) != 0, false};
        GameInput eventPacket{GameInputType::SetControls, packet->header.sessionId, inputs};
        _game.addInputQueue(std::vector<GameInput>{eventPacket});
    }
}

Session rtype::Server::createSession(asio::ip::udp::endpoint endpoint) {
    if (!_sessions.contains(endpoint)) {
        _sessions.insert({endpoint, Session{_idCounter}});
        _idCounter++;
    }

    return _sessions.at(endpoint);
}

Net::SnapshotPacket rtype::Server::convertGameOutput(GameOutput snapshot) const {
    Net::PacketHeader header;
    header.type = Net::PacketType::SNAPSHOT;

    Net::SnapshotPacket packet(header);
    std::vector<Net::EntityState> entities;

    for (GameEntityState state : snapshot.entities) {
        Net::EntityState statePacket;
        statePacket.x = state.position.x;
        statePacket.y = state.position.y;
        switch (state.kind) {
            case GameEntityKind::Player: {
                statePacket.kind = Net::EntityKind::PLAYER;
                break;
            } case GameEntityKind::Projectile: {
                statePacket.kind = Net::EntityKind::PROJECTILE;
                break;
            } case GameEntityKind::Enemy: {
                statePacket.kind = Net::EntityKind::ENEMY;
                break;
            } default:
                break;
        }
        statePacket.health = state.health;
        statePacket.id = state.id;
        entities.push_back(statePacket);
    }
    packet.entities = entities;
    return packet;
}

void rtype::Server::broadcast(Net::SnapshotPacket &packet) {
    std::cout << "send to all clients" << std::endl;
    for (const auto &[endpoint, session] : _sessions) {
        _netHandler.send(std::make_unique<Net::SnapshotPacket>(packet), endpoint);
    }
}