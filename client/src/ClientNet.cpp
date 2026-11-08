#include "ClientNet.hpp"
#include <iostream>

namespace {
    constexpr double REQUEST_TIMEOUT = 5.0;
    constexpr double RETRY_PERIOD = 0.5;
    constexpr double LIST_PERIOD = 1.0;
    constexpr double HEARTBEAT_PERIOD = 1.0;
}

rtype::ClientNet::ClientNet() : _wake(std::make_shared<WakeSignal>()), _network(0, _wake) {
    _thread = std::thread([this]() {
        try {
            _network.run();
        } catch (const std::exception &error) {
            std::cerr << "Network thread error : " << error.what() << '\n';
            _threadDied = true;
        }
    });
}

rtype::ClientNet::~ClientNet() {
    _network.stop();
    if (_thread.joinable())
        _thread.join();
}

double rtype::ClientNet::seconds(Clock::time_point point) const {
    return std::chrono::duration<double>(point - _start).count();
}

double rtype::ClientNet::now() const {
    return seconds(Clock::now());
}

std::string rtype::ClientNet::serverName() const {
    return _server.address().to_string() + ":" + std::to_string(_server.port());
}

void rtype::ClientNet::setState(State state) {
    _state = state;
    _stateSince = now();
    _lastTry = -1e9;
}

void rtype::ClientNet::fail(const std::string &message) {
    _error = message;
    setState(State::Failed);
}

bool rtype::ClientNet::connect(const std::string &address, const std::string &port, std::string &error) {
    asio::error_code code;
    const asio::ip::address_v4 ip = asio::ip::make_address_v4(address == "localhost" ? "127.0.0.1" : address, code);
    if (code) {
        error = "Invalid IPv4 adress : " + address;
        return false;
    }
    unsigned long value = 0;
    try {
        std::size_t used = 0;
        value = std::stoul(port, &used);
        if (used != port.size())
            value = 0;
    } catch (const std::exception &) {
        value = 0;
    }
    if (value < 1 || value > 65535) {
        error = "Invalid port (1-65535) : " + port;
        return false;
    }
    if (_threadDied) {
        error = "Network thread has stopped";
        return false;
    }
    _server = asio::ip::udp::endpoint(ip, static_cast<std::uint16_t>(value));
    _error.clear();
    _rooms.clear();
    _snapshots.clear();
    _events.clear();
    _chats.clear();
    _seenEvents.clear();
    _sessionId = 0;
    _instanceId = 0;
    _lastReceived = now();
    ++_roomsVersion;
    setState(State::Connecting);
    return true;
}

void rtype::ClientNet::createRoom() {
    if (_state != State::Lobby)
        return;
    _error.clear();
    _createRequested = true;
    setState(State::Joining);
}

void rtype::ClientNet::joinRoom(std::uint32_t id) {
    if (_state != State::Lobby)
        return;
    _error.clear();
    _createRequested = false;
    _joinTarget = id;
    setState(State::Joining);
}

void rtype::ClientNet::sendInput(std::uint16_t buttons) {
    if (_state != State::InGame)
        return;
    auto packet = std::make_unique<Net::InputPacket>();
    packet->header.sessionId = _sessionId;
    packet->buttons = buttons;
    _network.send(std::move(packet), _server);
    _lastButtons = buttons;
    _lastInput = now();
}

void rtype::ClientNet::sendChat(const std::string &text) {
    if (_state != State::InGame || text.empty())
        return;
    const std::string payload = text.size() > Net::MAX_CHAT_LENGTH
        ? text.substr(0, Net::MAX_CHAT_LENGTH)
        : text;
    auto packet = std::make_unique<Net::EventPacket>();
    packet->header.sessionId = _sessionId;
    packet->eventType = Net::EventType::CHAT;
    packet->eventData = std::make_shared<Net::ChatEvent>(_sessionId, payload);
    _network.send(std::move(packet), _server);
}

void rtype::ClientNet::sendAck(std::uint32_t eventId) {
    auto ack = std::make_unique<Net::EventPacket>();
    ack->header.sessionId = _sessionId;
    ack->eventType = Net::EventType::ACK;
    ack->eventId = eventId;
    _network.send(std::move(ack), _server);
}

void rtype::ClientNet::resend() {
    const double current = now();

    switch (_state) {
        case State::Connecting:
            if (current - _stateSince > REQUEST_TIMEOUT) {
                fail("Server " + serverName() + " is not responding.");
            } else if (current - _lastTry >= RETRY_PERIOD) {
                _lastTry = current;
                _network.send(std::make_unique<Net::HelloPacket>(), _server);
            }
            break;
        case State::Lobby:
            if (current - _lastReceived > REQUEST_TIMEOUT) {
                fail("Connection lost.");
            } else if (current - _lastList >= LIST_PERIOD) {
                _lastList = current;
                auto packet = std::make_unique<Net::ListInstancesPacket>();
                packet->header.sessionId = _sessionId;
                _network.send(std::move(packet), _server);
            }
            break;
        case State::Joining:
            if (current - _stateSince > REQUEST_TIMEOUT) {
                fail("Server is not responding.");
            } else if (current - _lastTry >= RETRY_PERIOD) {
                _lastTry = current;
                if (_createRequested) {
                    auto packet = std::make_unique<Net::CreateInstancePacket>();
                    packet->header.sessionId = _sessionId;
                    _network.send(std::move(packet), _server);
                } else {
                    auto packet = std::make_unique<Net::InstancePacket>();
                    packet->header.sessionId = _sessionId;
                    packet->instanceId = _joinTarget;
                    _network.send(std::move(packet), _server);
                }
            }
            break;
        case State::InGame:
            if (current - _lastReceived > REQUEST_TIMEOUT)
                fail("Server connection lost.");
            else if (current - _lastInput >= HEARTBEAT_PERIOD)
                sendInput(_lastButtons);
            break;
        default:
            break;
    }
}

void rtype::ClientNet::handleInstanceReply(std::uint32_t instanceId) {
    if (_state != State::Joining)
        return;
    if (instanceId == Net::NO_INSTANCE) {
        _error = "Room not found or already joined.";
        setState(State::Lobby);
        return;
    }
    _instanceId = instanceId;
    _chats.clear();
    _seenEvents.clear();
    _lastReceived = now();
    _lastInput = now();
    _lastButtons = 0;
    setState(State::InGame);
}

void rtype::ClientNet::handleEvent(const Net::EventPacket &event) {
    sendAck(event.eventId);
    if (_state != State::InGame)
        return;
    if (!_seenEvents.insert(event.eventId).second)
        return;
    if (event.eventType == Net::EventType::CHAT) {
        if (const auto *chat = dynamic_cast<const Net::ChatEvent *>(event.eventData.get()))
            _chats.push_back({chat->senderId, chat->text});
        return;
    }
    auto *entity = dynamic_cast<const Net::EntityEvent *>(event.eventData.get());
    if (!entity)
        return;
    if (event.eventType == Net::EventType::DESTROY || event.eventType == Net::EventType::YOUR_ENTITY)
        _events.push_back({event.eventType, entity->entityId});
}

void rtype::ClientNet::pump() {
    while (auto incoming = _network.popPacket()) {
        auto message = std::move(*incoming);
        auto *packet = message.packet.get();

        if (!packet || message.sender != _server)
            continue;
        _lastReceived = now();
        if (auto *welcome = dynamic_cast<Net::WelcomePacket *>(packet)) {
            if (_state == State::Connecting) {
                _sessionId = welcome->clientId;
                setState(State::Lobby);
                _lastList = -1e9;
            }
        } else if (auto *list = dynamic_cast<Net::InstanceListPacket *>(packet)) {
            if (_state == State::Lobby || _state == State::Joining) {
                _rooms.clear();
                for (const auto &info : list->instances)
                    _rooms.push_back({info.id, info.players});
                ++_roomsVersion;
            }
        } else if (auto *reply = dynamic_cast<Net::InstancePacket *>(packet)) {
            handleInstanceReply(reply->instanceId);
        } else if (auto *snapshot = dynamic_cast<Net::SnapshotPacket *>(packet)) {
            if (_state == State::InGame)
                _snapshots.push_back({snapshot->serverTick, snapshot->score, seconds(message.receivedAt), std::move(snapshot->entities)});
        } else if (auto *event = dynamic_cast<Net::EventPacket *>(packet)) {
            handleEvent(*event);
        }
    }
}

void rtype::ClientNet::update() {
    if (_threadDied && _state != State::Failed && _state != State::Idle)
        fail("network thread has stopped.");
    pump();
    resend();
}

std::vector<rtype::ClientNet::Snapshot> rtype::ClientNet::takeSnapshots() {
    std::vector<Snapshot> out;
    out.swap(_snapshots);
    return out;
}

std::vector<rtype::ClientNet::Event> rtype::ClientNet::takeEvents() {
    std::vector<Event> out;
    out.swap(_events);
    return out;
}

std::vector<rtype::ClientNet::ChatMessage> rtype::ClientNet::takeChats() {
    std::vector<ChatMessage> out;
    out.swap(_chats);
    return out;
}
