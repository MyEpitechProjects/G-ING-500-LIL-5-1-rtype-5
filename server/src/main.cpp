#include "Server.hpp"

int main() {
    rtype::Server server(4242);
    server.run();
}