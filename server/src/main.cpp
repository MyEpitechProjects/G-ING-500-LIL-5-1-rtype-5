#include "Server.hpp"

int main() {
    try {
        rtype::Server server(4242, Net::SERVER_TICK_RATE);
        server.run();
        return 0;
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 84;
    }
    
}