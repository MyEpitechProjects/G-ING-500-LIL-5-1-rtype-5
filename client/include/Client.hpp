#pragma once
#include "NetworkHandler.hpp"

namespace rtype {
    class Client {
        private:
            Net::NetworkHandler _netHandler;
            
        public:
            Client(std::string adress, int port);
            ~Client();

            void run();
    };
}
