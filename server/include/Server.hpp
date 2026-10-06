#ifndef SERVER_HPP
    #define SERVER_HPP

    #include "Receiver.hpp"

namespace rtype {
    class Server {
        private:
            Net::Receiver _receiver;
        public:
            Server(int port);
            ~Server();
    };

    Server::Server(int port) {
    
    }

    Server::~Server() {
    
    }
    
}

#endif