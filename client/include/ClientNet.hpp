#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <asio/ip/udp.hpp>
#include "NetworkHandler.hpp"
#include "WakeSignal.hpp"

namespace rtype {
    class ClientNet {
        public:
            enum class State { Idle, Connecting, Lobby, Joining, InGame, Failed };

            struct Room {
                std::uint32_t id;
                std::uint16_t players;
            };
            struct Snapshot {
                std::uint32_t tick;
                double receivedAt;
                std::vector<Net::EntityState> entities;
            };
            struct Event {
                Net::EventType type;
                std::uint32_t entityId;
            };

            ClientNet();
            ~ClientNet();
            ClientNet(const ClientNet &) = delete;
            ClientNet &operator=(const ClientNet &) = delete;

            bool connect(const std::string &address, const std::string &port, std::string &error);
            void createRoom();
            void joinRoom(std::uint32_t id);
            void sendInput(std::uint16_t buttons);
            void update();

            State state() const { return _state; }
            const std::string &error() const { return _error; }
            const std::vector<Room> &rooms() const { return _rooms; }
            unsigned roomsVersion() const { return _roomsVersion; }
            std::uint32_t instanceId() const { return _instanceId; }
            std::string serverName() const;
            double now() const;

            std::vector<Snapshot> takeSnapshots();
            std::vector<Event> takeEvents();

        private:
            using Clock = std::chrono::steady_clock;

            std::shared_ptr<WakeSignal> _wake;
            Net::NetworkHandler _network;
            std::thread _thread;
            std::atomic<bool> _threadDied{false};
            Clock::time_point _start = Clock::now();

            asio::ip::udp::endpoint _server;
            State _state = State::Idle;
            std::string _error;
            std::uint32_t _sessionId = 0;
            std::uint32_t _instanceId = 0;
            bool _createRequested = true;
            std::uint32_t _joinTarget = 0;
            std::uint16_t _lastButtons = 0;

            double _stateSince = 0;
            double _lastTry = -1e9;
            double _lastList = -1e9;
            double _lastInput = 0;
            double _lastReceived = 0;

            std::vector<Room> _rooms;
            unsigned _roomsVersion = 0;
            std::vector<Snapshot> _snapshots;
            std::vector<Event> _events;

            void setState(State state);
            void fail(const std::string &message);
            void pump();
            void resend();
            void sendAck(std::uint32_t eventId);
            void handleInstanceReply(std::uint32_t instanceId);
            void handleEvent(const Net::EventPacket &event);
            double seconds(Clock::time_point point) const;
    };
}
