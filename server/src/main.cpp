#include "Server.hpp"

int main() {
    rtype::Server server(4242, Net::SERVER_TICK_RATE);
    server.run();
    return 0;
}