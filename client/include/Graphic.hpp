#pragma once
#include "AssetsPak.hpp"
#include "ObjectManager.hpp"
#include <asio/ip/udp.hpp>
#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>

namespace rtype {
    class Graphic {
        private:
            bool init = false;
            sf::RenderWindow window;
            AssetsPak assets;
            ObjectManager object_manager;
            bool is_connected();
        public:
            Graphic(uint16_t width, uint16_t height, std::string title, std::string assets_file, asio::ip::udp::endpoint server_endpoint);
            ~Graphic();
            void handle_objects(rtype::InputKeys keys);
            void run();
    };
}
