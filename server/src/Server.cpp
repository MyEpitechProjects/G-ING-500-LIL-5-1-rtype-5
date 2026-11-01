#include "Server.hpp"

rtype::Server::Server(int port, std::size_t tickRate) : _wakeSignal(std::make_shared<WakeSignal>()), _netHandler(port, _wakeSignal), _pool(std::thread::hardware_concurrency()), _idCounterSession(0), _idEventCounter(0), _idInstanceCounter(1) {
    _tickPeriod = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(1.0 / tickRate)
    );
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

    while (_running) {
        auto nextTick = std::chrono::steady_clock::now() + _tickPeriod;

        removeInactiveClients();

        // packet received :

        while (auto packet = _netHandler.popPacket()) {
            handlePacket(packet.value());
        }

        /*std::vector<GameOutput> outputs = _game.takeOutputQueue();
        for (GameOutput output : outputs) {
            if (output.type == GameOutputType::Snapshot) {
                Net::SnapshotPacket snapPacket = convertGameOutput(output);
                broadcast(std::make_shared<Net::SnapshotPacket>(snapPacket));
            } else if (output.type == GameOutputType::PlayerCreated) {
                auto sessionIt = std::find_if(
                    _sessions.begin(),
                    _sessions.end(),
                    [&output](const auto& entry) {
                        return entry.second.client_id == output.clientId;
                    }
                );
                if (sessionIt != _sessions.end()) {
                    Net::WelcomePacket welcome;
                    welcome.header.sessionId = sessionIt->second.client_id;
                    welcome.playerEntityId = output.clientId;
                    welcome.entityId = output.entityId;
                    _netHandler.send(
                        std::make_unique<Net::WelcomePacket>(welcome),
                        sessionIt->first
                    );
                }
            }
            if (output.type == GameOutputType::EntityDestroyed || output.type == GameOutputType::PlayerCreated) {
                Net::EventPacket outpacket;
                outpacket.header.sessionId = output.clientId;
                outpacket.eventType = output.type == GameOutputType::EntityDestroyed ? Net::EventType::DESTROY : Net::EventType::NEW_CLIENT;
                outpacket.eventId = _idEventCounter;
                _idEventCounter++;
                outpacket.entityId = output.entityId;
                broadcastEvent(outpacket);
            }
        }

        handlePendingEvents();*/

        // wait next tick, define by tick rate :
        _wakeSignal->waitUntil(nextTick);
    }
}

void rtype::Server::handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet> &incomingPacket) {
    if (auto packet = dynamic_cast<Net::HelloPacket*>(incomingPacket.packet.get())) {
        handlePacket(*packet, incomingPacket.sender);
    } else if (auto packet = dynamic_cast<Net::CreateInstancePacket*>(incomingPacket.packet.get())) {
        handlePacket(*packet, incomingPacket.sender);
    } else if (auto packet = dynamic_cast<Net::InstancePacket*>(incomingPacket.packet.get())) {
        handlePacket(*packet, incomingPacket.sender);
    } else if (dynamic_cast<Net::InputPacket*>(incomingPacket.packet.get())) {
        //Controllable inputs{(packet->buttons & Net::InputButton::UP) != 0, (packet->buttons & Net::InputButton::DOWN) != 0, (packet->buttons & Net::InputButton::RIGHT) != 0, (packet->buttons & Net::InputButton::LEFT) != 0, (packet->buttons & Net::InputButton::FIRE) != 0};
        //if (_sessions.find(incomingPacket.sender) != _sessions.end() && _sessions.at(incomingPacket.sender).client_id == packet->header.sessionId) {
        //    GameInput eventPacket{GameInputType::SetControls, packet->header.sessionId, inputs};
        //    _game.addInputQueue(std::vector<GameInput>{eventPacket});
        //}
    } else if (auto packet = dynamic_cast<Net::EventPacket*>(incomingPacket.packet.get())) {
        if (packet->eventType == Net::EventType::ACK) {
            erasePendingEvent(incomingPacket.sender, packet->eventId, packet->header.sessionId);
        }
    }
    if (_sessions.find(incomingPacket.sender) != _sessions.end())
        _sessions.at(incomingPacket.sender).lastSeen = std::chrono::steady_clock::now();
}

void rtype::Server::handlePacket(Net::HelloPacket &, Net::udp::endpoint sender) {
    const Session &playerSession = createSession(sender);
    Net::WelcomePacket welcome;
    welcome.header.sessionId = static_cast<std::uint32_t>(playerSession.client_id);
    welcome.clientId = static_cast<std::uint32_t>(playerSession.client_id);
    _netHandler.send(std::make_unique<Net::WelcomePacket>(welcome), sender);
}

void rtype::Server::handlePacket(Net::CreateInstancePacket &, Net::udp::endpoint sender) {
    if (_sessions.find(sender) != _sessions.end()) {
        std::uint32_t newInstanceId = createInstance();
        Net::InstancePacket instancePacket;
        instancePacket.instanceId = newInstanceId;
        _sessions.at(sender).instance_id = instancePacket.instanceId;
        _netHandler.send(std::make_unique<Net::InstancePacket>(instancePacket), sender);

        _pool.addInQueue([this, newInstanceId](){
            std::unordered_map<uint32_t, Instance>::node_type node = [this, newInstanceId]() {
                std::lock_guard<std::mutex> lock(_instancesMutex);
                return _instances.extract(newInstanceId);
            }();

            if (node.empty())
                return;

            [[maybe_unused]] Instance &instance = node.mapped();
            // traitement de l'instance

            {
                std::lock_guard<std::mutex> lock(_instancesMutex);
                _instances.insert(std::move(node));
            }
        });
    }
}

void rtype::Server::handlePacket(Net::InstancePacket &packet, Net::udp::endpoint sender) {
    auto sessionIt = _sessions.find(sender);
    if (sessionIt == _sessions.end() || sessionIt->second.client_id != packet.header.sessionId)
        return;

    Net::InstancePacket response;
    response.header.sessionId = packet.header.sessionId;
    response.instanceId = joinInstance(sessionIt->second, packet.instanceId) ? packet.instanceId : Net::NO_INSTANCE;
    _netHandler.send(std::make_unique<Net::InstancePacket>(response), sender);
}

bool rtype::Server::joinInstance(Session &session, std::uint32_t instanceId) {
    if (instanceId == Net::NO_INSTANCE)
        return false;
    if (session.instance_id == instanceId)
        return true;
    if (session.instance_id != Net::NO_INSTANCE)
        return false;

    {
        std::lock_guard<std::mutex> lock(_instancesMutex);
        auto instanceIt = _instances.find(instanceId);
        if (instanceIt == _instances.end())
            return false;
        instanceIt->second.addPlayer(session.client_id);
    }
    session.instance_id = instanceId;
    return true;
}

Session &rtype::Server::createSession(asio::ip::udp::endpoint endpoint) {
    if (!_sessions.contains(endpoint)) {
        Session session;
        session.client_id = _idCounterSession;
        _sessions.insert({endpoint, session});
        _idCounterSession++;
    }

    return _sessions.at(endpoint);
}

std::uint32_t rtype::Server::createInstance() {
    const std::uint32_t instanceId = static_cast<std::uint32_t>(_idInstanceCounter);
    {
        std::lock_guard<std::mutex> lock(_instancesMutex);
        _instances.try_emplace(instanceId, _wakeSignal, instanceId);
    }
    _idInstanceCounter++;

    return instanceId;
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

void rtype::Server::broadcastEvent(Net::EventPacket outPacket) {
    const auto now = std::chrono::steady_clock::now();
    for (auto &[endpoint, session] : _sessions) {
        outPacket.header.sessionId = session.client_id;
        session.pendingEvents.insert({outPacket.eventId, PendingEvent{std::make_shared<Net::EventPacket>(outPacket), now}});
    }
}

void rtype::Server::removeInactiveClients() {
    const auto now = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(10);

    for (auto it = _sessions.begin(); it != _sessions.end();) {
        if (now - it->second.lastSeen >= timeout) {
            std::cout << "remove player : " << it->second.client_id << std::endl;
            leaveInstance(it->second);

            Net::EventPacket disconnectPacket;
            disconnectPacket.eventType = Net::EventType::DISCONNECTED;
            disconnectPacket.eventId = static_cast<std::uint32_t>(_idEventCounter);
            disconnectPacket.eventData = std::make_shared<Net::EntityEvent>(static_cast<std::uint32_t>(it->second.client_id));
            _idEventCounter++;

            it = _sessions.erase(it);
            broadcastEvent(disconnectPacket);
        } else {
            ++it;
        }
    }
}

void rtype::Server::leaveInstance(Session &session) {
    if (session.instance_id == Net::NO_INSTANCE)
        return;

    std::lock_guard<std::mutex> lock(_instancesMutex);
    auto instanceIt = _instances.find(static_cast<std::uint32_t>(session.instance_id));
    if (instanceIt != _instances.end())
        instanceIt->second.removePlayer(session.client_id);
    session.instance_id = Net::NO_INSTANCE;
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