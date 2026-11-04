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

    auto nextTick = std::chrono::steady_clock::now();

    while (_running) {
        removeInactiveClients();

        // packet received :

        while (auto packet = _netHandler.popPacket()) {
            handlePacket(packet.value());
        }

        // update instance :

        const auto now = std::chrono::steady_clock::now();
        if (now >= nextTick) {
            updateInstances();
            nextTick += _tickPeriod;
            if (nextTick <= now)
                nextTick = now + _tickPeriod;
        }

        dispatchOutputs();
        handlePendingEvents();

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
    } else if (auto packet = dynamic_cast<Net::InputPacket*>(incomingPacket.packet.get())) {
        handlePacket(*packet, incomingPacket.sender);
    } else if (auto packet = dynamic_cast<Net::EventPacket*>(incomingPacket.packet.get())) {
        handlePacket(*packet, incomingPacket.sender);
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
    auto sessionIt = _sessions.find(sender);
    if (sessionIt == _sessions.end())
        return;

    Session &session = sessionIt->second;
    Net::InstancePacket response;
    response.header.sessionId = static_cast<std::uint32_t>(session.client_id);
    response.instanceId = Net::NO_INSTANCE;
    if (session.instance_id == Net::NO_INSTANCE) {
        const std::uint32_t newInstanceId = createInstance();
        if (joinInstance(session, newInstanceId))
            response.instanceId = newInstanceId;
    }
    _netHandler.send(std::make_unique<Net::InstancePacket>(response), sender);
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

void rtype::Server::handlePacket(Net::InputPacket &packet, Net::udp::endpoint sender) {
    auto sessionIt = _sessions.find(sender);
    if (sessionIt == _sessions.end() || sessionIt->second.client_id != packet.header.sessionId)
        return;

    const Session &session = sessionIt->second;
    if (session.instance_id == Net::NO_INSTANCE)
        return;

    const Controllable controls{
        (packet.buttons & Net::InputButton::UP) != 0,
        (packet.buttons & Net::InputButton::DOWN) != 0,
        (packet.buttons & Net::InputButton::RIGHT) != 0,
        (packet.buttons & Net::InputButton::LEFT) != 0,
        (packet.buttons & Net::InputButton::FIRE) != 0
    };

    std::lock_guard<std::mutex> lock(_instancesMutex);
    auto instanceIt = _instances.find(static_cast<std::uint32_t>(session.instance_id));
    if (instanceIt != _instances.end())
        instanceIt->second.setControls(session.client_id, controls);
}

void rtype::Server::handlePacket(Net::EventPacket &packet, Net::udp::endpoint sender) {
    switch (packet.eventType) {
        case Net::EventType::ACK:
            erasePendingEvent(sender, packet.eventId, packet.header.sessionId);
            break;
        case Net::EventType::CHAT:
            if (auto chat = dynamic_cast<const Net::ChatEvent*>(packet.eventData.get()))
                handleChat(*chat, sender, packet.header.sessionId);
            break;
        default:
            break;
    }
}

void rtype::Server::handleChat(const Net::ChatEvent &chat, Net::udp::endpoint sender, std::uint32_t sessionId) {
    auto sessionIt = _sessions.find(sender);
    if (sessionIt == _sessions.end() || sessionIt->second.client_id != sessionId)
        return;

    const Session &session = sessionIt->second;
    if (session.instance_id == Net::NO_INSTANCE || chat.text.empty())
        return;

    broadcastEvent(static_cast<std::uint32_t>(session.instance_id), createChatEvent(session.client_id, chat.text));
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
        _instances.try_emplace(instanceId, instanceId);
    }
    _idInstanceCounter++;

    return instanceId;
}

Net::SnapshotPacket rtype::Server::convertGameOutput(const GameOutput &snapshot) const {
    Net::PacketHeader header;
    header.type = Net::PacketType::SNAPSHOT;

    Net::SnapshotPacket packet(header);
    std::vector<Net::EntityState> entities;

    for (const GameEntityState &state : snapshot.entities) {
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

void rtype::Server::updateInstances() {
    std::lock_guard<std::mutex> lock(_instancesMutex);
    for (auto &[instanceId, instance] : _instances) {
        if (!instance.tryStartUpdate())
            continue;
        Instance *target = &instance;
        _pool.addInQueue([target]() { target->update(); });
    }
}

void rtype::Server::dispatchOutputs() {
    std::vector<std::pair<std::uint32_t, GameOutput>> outputs;
    {
        std::lock_guard<std::mutex> lock(_instancesMutex);
        for (auto &[instanceId, instance] : _instances) {
            for (GameOutput &output : instance.takeOutputs())
                outputs.emplace_back(instanceId, std::move(output));
        }
    }

    for (const auto &[instanceId, output] : outputs)
        dispatchOutput(instanceId, output);
}

void rtype::Server::dispatchOutput(std::uint32_t instanceId, const GameOutput &output) {
    switch (output.type) {
        case GameOutputType::Snapshot:
            sendToInstance(instanceId, std::make_shared<Net::SnapshotPacket>(convertGameOutput(output)));
            break;
        case GameOutputType::PlayerCreated:
            broadcastEvent(instanceId, createEntityEvent(Net::EventType::NEW_CLIENT, output.entityId));
            break;
        case GameOutputType::EntityDestroyed:
            broadcastEvent(instanceId, createEntityEvent(Net::EventType::DESTROY, output.entityId));
            break;
    }
}

Net::EventPacket rtype::Server::createEntityEvent(Net::EventType type, std::size_t entityId) {
    Net::EventPacket packet;
    packet.eventType = type;
    packet.eventId = static_cast<std::uint32_t>(_idEventCounter);
    packet.eventData = std::make_shared<Net::EntityEvent>(static_cast<std::uint32_t>(entityId));
    _idEventCounter++;
    return packet;
}

Net::EventPacket rtype::Server::createChatEvent(std::size_t senderId, const std::string &text) {
    Net::EventPacket packet;
    packet.eventType = Net::EventType::CHAT;
    packet.eventId = static_cast<std::uint32_t>(_idEventCounter);
    packet.eventData = std::make_shared<Net::ChatEvent>(static_cast<std::uint32_t>(senderId), text);
    _idEventCounter++;
    return packet;
}

void rtype::Server::sendToInstance(std::uint32_t instanceId, Net::PacketCodec::Packet packet) {
    for (const auto &[endpoint, session] : _sessions) {
        if (session.instance_id == instanceId)
            _netHandler.send(packet, endpoint);
    }
}

void rtype::Server::broadcastEvent(std::uint32_t instanceId, Net::EventPacket outPacket) {
    const auto now = std::chrono::steady_clock::now();
    for (auto &[endpoint, session] : _sessions) {
        if (session.instance_id != instanceId)
            continue;
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
            const std::uint32_t instanceId = static_cast<std::uint32_t>(it->second.instance_id);
            const std::size_t clientId = it->second.client_id;
            leaveInstance(it->second);

            it = _sessions.erase(it);
            if (instanceId != Net::NO_INSTANCE)
                broadcastEvent(instanceId, createEntityEvent(Net::EventType::DISCONNECTED, clientId));
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