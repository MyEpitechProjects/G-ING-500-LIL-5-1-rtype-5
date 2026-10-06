#include "Server.hpp"

rtype::Server::Server(int port) : _netHandler(port) {

}

rtype::Server::~Server() {

}

void rtype::Server::run() {
    _netHandler.run();
}