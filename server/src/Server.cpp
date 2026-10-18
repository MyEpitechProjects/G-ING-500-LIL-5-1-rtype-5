#include "Server.hpp"

rtype::Server::Server(int port) : _wakeSignal(std::move(std::make_shared<WakeSignal>())), _netHandler(port, _wakeSignal), _game(_wakeSignal), _idCounter(0), _idEventCounter(0) {

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
        _wakeSignal->waitFor(std::chrono::milliseconds(500));

        removeInactiveClients();

        while (auto packet = _netHandler.popPacket()) {
            std::cout << "network packet received" << std::endl;
            handlePacket(packet.value());
        }

        std::vector<GameOutput> outputs = _game.takeOutputQueue();
        for (GameOutput output : outputs) {
            if (output.type == GameOutputType::Snapshot) {
                Net::SnapshotPacket snapPacket = convertGameOutput(output);
                broadcast(std::move(std::make_shared<Net::SnapshotPacket>(snapPacket)));
            } else if (output.type == GameOutputType::EntityDestroyed || output.type == GameOutputType::PlayerCreated) {
                Net::GameEventPacket outpacket;
                outpacket.header.sessionId = output.clientId;
                outpacket.eventType = output.type == GameOutputType::EntityDestroyed ? Net::EventType::DESTROY : Net::EventType::NEW_CLIENT;
                outpacket.eventId = _idEventCounter;
                _idEventCounter++;
                outpacket.entityId = output.entityId;
                broadcastEvent(outpacket);
            }
        }

        handlePendingEvents();
    }
}

void rtype::Server::handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet> &incomingPacket) {
    if (dynamic_cast<Net::HelloPacket*>(incomingPacket.packet.get())) {
        Session playerSession = createSession(incomingPacket.sender);
        Net::WelcomePacket weclome;
        weclome.header.sessionId = playerSession.client_id;
        weclome.playerEntityId = playerSession.client_id;
        GameInput addPlayer{GameInputType::AddPlayer, playerSession.client_id};
        _game.addInputQueue(std::vector<GameInput>{addPlayer});
        std::unique_ptr<Net::WelcomePacket> packet = std::make_unique<Net::WelcomePacket>(weclome);
        _netHandler.send(std::move(packet), incomingPacket.sender);
    } else if (auto packet = dynamic_cast<Net::InputPacket*>(incomingPacket.packet.get())) {
        Controllable inputs{(packet->buttons & Net::InputButton::UP) != 0, (packet->buttons & Net::InputButton::DOWN) != 0, (packet->buttons & Net::InputButton::RIGHT) != 0, (packet->buttons & Net::InputButton::LEFT) != 0, (packet->buttons & Net::InputButton::FIRE) != 0};
        if (_sessions.find(incomingPacket.sender) != _sessions.end() && _sessions.at(incomingPacket.sender).client_id == packet->header.sessionId) {
            GameInput eventPacket{GameInputType::SetControls, packet->header.sessionId, inputs};
            _game.addInputQueue(std::vector<GameInput>{eventPacket});
        }
    } else if (auto packet = dynamic_cast<Net::GameEventPacket*>(incomingPacket.packet.get())) {
        if (packet->eventType == Net::EventType::ACK) {
            erasePendingEvent(incomingPacket.sender, packet->eventId, packet->header.sessionId);
        }
    }
    if (_sessions.find(incomingPacket.sender) != _sessions.end())
        _sessions.at(incomingPacket.sender).lastSeen = std::chrono::steady_clock::now();
}

Session rtype::Server::createSession(asio::ip::udp::endpoint endpoint) {
    if (!_sessions.contains(endpoint)) {
        _sessions.insert({endpoint, Session{_idCounter, {}}});
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
    packet.serverTick = snapshot.tick;
    return packet;
}

void rtype::Server::broadcast(Net::PacketCodec::Packet packet) {
    for (const auto &[endpoint, session] : _sessions) {
        _netHandler.send(packet, endpoint);
    }
}

void rtype::Server::broadcastEvent(Net::GameEventPacket outPacket) {
    const auto now = std::chrono::steady_clock::now();
    for (auto &[endpoint, session] : _sessions) {
        outPacket.header.sessionId = session.client_id;
        session.pendingEvents.insert({outPacket.eventId, PendingEvent{std::make_shared<Net::GameEventPacket>(outPacket), now}});
    }
}

void rtype::Server::removeInactiveClients() {
    const auto now = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(10);
    std::vector<GameInput> removeList;

    for (auto it = _sessions.begin(); it != _sessions.end();) {
        if (now - it->second.lastSeen >= timeout) {
            std::cout << "remove player : " << it->second.client_id << std::endl;
            GameInput remover{GameInputType::RemovePlayer, it->second.client_id};
            removeList.push_back(remover);

            Net::GameEventPacket disconnectPacket;
            disconnectPacket.eventType = Net::EventType::DISCONNECTED;
            disconnectPacket.entityId = it->second.client_id;
            disconnectPacket.eventId = _idEventCounter;
            _idEventCounter++;

            it = _sessions.erase(it);
            broadcastEvent(disconnectPacket);
        } else {
            ++it;
        }
    }
    _game.addInputQueue(std::vector<GameInput>(removeList));
}

void rtype::Server::insertPendingEvent(std::size_t client_id, PendingEvent event, std::uint32_t event_id) {
    for (auto it = _sessions.begin(); it != _sessions.end();) {
        if (it->second.client_id == client_id) {
            it->second.pendingEvents.insert({event_id, event});
            return;
        } else {
            ++it;
        }
    }
}

void rtype::Server::erasePendingEvent(asio::ip::udp::endpoint sender, std::uint32_t event_id, std::size_t sessionId) {
    if (_sessions.find(sender) != _sessions.end() && _sessions.at(sender).pendingEvents.find(event_id) != _sessions.at(sender).pendingEvents.end()
        && _sessions.at(sender).client_id == sessionId) {
        _sessions.at(sender).pendingEvents.erase(event_id);
    }
}

void rtype::Server::handlePendingEvents() {
    const auto now = std::chrono::steady_clock::now();
    for (auto it = _sessions.begin(); it != _sessions.end();) {
        for (auto it_second = it->second.pendingEvents.begin(); it_second != it->second.pendingEvents.end();) {
            if (now >= it_second->second.nextRetry) {
                _netHandler.send(it_second->second.packet, it->first);
                it_second->second.attempts++;
                it_second->second.nextRetry = now + std::chrono::milliseconds(200);
            }
            it_second++;
        }
        it++;
    }
}