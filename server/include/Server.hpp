#ifndef SERVER_HPP
    #define SERVER_HPP

    #include "NetworkHandler.hpp"
    #include "Session.hpp"
    #include "WakeSignal.hpp"
    #include "Game.hpp"

    #include <thread>
    #include <queue>
    #include <memory>

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

            void handlePacket(Net::IncomingPacket<Net::PacketCodec::Packet>& incomingPacket);

            Session createSession(asio::ip::udp::endpoint endpoint);
            
        public:
            Server(int port);
            ~Server();

            void run();
    };
}

#endif