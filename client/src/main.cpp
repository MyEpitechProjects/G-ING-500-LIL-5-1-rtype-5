#include "Graphic.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [server IPv4 address]\n";
        return 1;
    }

    const char* server_ip = argc == 2 ? argv[1] : "127.0.0.1";
    asio::error_code error;
    const asio::ip::address_v4 address = asio::ip::make_address_v4(server_ip, error);
    if (error) {
        std::cerr << "Invalid server IPv4 address: " << server_ip << '\n';
        return 1;
    }

    rtype::Graphic graphic(800, 600, "R-Type", "assets.pak",
        asio::ip::udp::endpoint(address, 4242));
    graphic.run();
    return 0;
}
