#include "Server.hpp"

int main() {
    rtype::Server server(4242, 20);
    server.run();
    return 0;
}