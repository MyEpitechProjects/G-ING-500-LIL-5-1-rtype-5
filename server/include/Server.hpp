#ifndef SERVER_HPP
    #define SERVER_HPP

    #include "NetworkHandler.hpp"
    #include "Session.hpp"

namespace rtype {
    class Server {
        private:
            Net::NetworkHandler _netHandler;
            std::unordered_map<asio::ip::udp::endpoint, Session> _sessions;
            
        public:
            Server(int port);
            ~Server();

            void run();
    };
}

#endif