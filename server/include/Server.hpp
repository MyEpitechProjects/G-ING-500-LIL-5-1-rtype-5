#ifndef SERVER_HPP
    #define SERVER_HPP

    #include "NetworkHandler.hpp"
    #include "Session.hpp"
    #include "WakeSignal.hpp"
    #include "Game.hpp"

    #include <thread>
    #include <queue>
    #include <memory>
    #include <ranges>

namespace rtype {
    class Server {
        private:
            bool _running = true;
            std::shared_ptr<WakeSignal> _wakeSignal;

            Net::NetworkHandler _netHandler;
            Game _game;
            std::thread _netThread;
            std::thread _gameThread;

            std::unordered_map<asio::ip::udp::endpoint, Session> _sessions;
            std::size_t _idCounter;
            std::size_t _idEventCounter;

            void handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet>& incomingPacket);

            Session createSession(asio::ip::udp::endpoint endpoint);

            Net::SnapshotPacket convertGameOutput(GameOutput snapshot) const;

            void broadcast(Net::PacketCodec::Packet packet);

            void removeInactiveClients();

            void insertPendingEvent(std::size_t client_id, PendingEvent event, std::uint32_t event_id);
            void broadcastEvent(Net::GameEventPacket outPacket);
            void erasePendingEvent(asio::ip::udp::endpoint sender, std::uint32_t event_id, std::size_t sessionId);

            void handlePendingEvents();
            
        public:
            Server(int port);
            ~Server();

            void run();
    };
}

#endif