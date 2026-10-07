#include "Server.hpp"

rtype::Server::Server(int port) : _netHandler(port) {

}

rtype::Server::~Server() {
    _netHandler.stop();
}

void rtype::Server::run() {
    _netHandler.run();    
}