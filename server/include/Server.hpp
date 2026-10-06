#ifndef SERVER_HPP
    #define SERVER_HPP

    #include "NetworkHandler.hpp"

namespace rtype {
    class Server {
        private:
            Net::NetworkHandler _netHandler;
            
        public:
            Server(int port);
            ~Server();
    };

    Server::Server(int port) : _netHandler(port) {
    
    }

    Server::~Server() {
    
    }
    
}

#endif