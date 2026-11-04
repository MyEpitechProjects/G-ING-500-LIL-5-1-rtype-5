#include "App.hpp"
#include <iostream>
#include <string>

static int usage(const char *name) {
    std::cerr << "Usage: " << name
              << " [IPv4 address[:port]] [port] [--create | --join <room ID>]\n"
              << "  Without options, the connection window opens (address, port, then room selection).\n"
              << "  --create / --join connect directly and skip the lobby.\n";
    return 1;
}

int main(int argc, char *argv[]) {
    rtype::App::Options options;
    int positional = 0;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--create") {
            options.autoConnect = true;
            options.autoCreate = true;
            options.autoJoin = 0;
        } else if (arg == "--join") {
            if (i + 1 >= argc)
                return usage(argv[0]);
            unsigned long id = 0;
            try {
                id = std::stoul(argv[++i]);
            } catch (const std::exception &) {
                return usage(argv[0]);
            }
            if (id == 0)
                return usage(argv[0]);
            options.autoConnect = true;
            options.autoCreate = false;
            options.autoJoin = static_cast<std::uint32_t>(id);
        } else if (!arg.empty() && arg[0] != '-') {
            if (positional == 0) {
                const std::size_t colon = arg.find(':');
                options.address = arg.substr(0, colon);
                if (colon != std::string::npos)
                    options.port = arg.substr(colon + 1);
            } else if (positional == 1) {
                options.port = arg;
            } else {
                return usage(argv[0]);
            }
            ++positional;
        } else {
            return usage(argv[0]);
        }
    }
    try {
        rtype::App app(options);
        app.run();
    } catch (const std::exception &error) {
        std::cerr << "Error : " << error.what() << '\n';
        return 1;
    }
    return 0;
}
