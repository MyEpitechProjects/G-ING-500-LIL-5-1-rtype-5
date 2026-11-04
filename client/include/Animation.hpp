#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>
#include "AssetsPak.hpp"

namespace rtype {
    struct Frame {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
    };

    class Animation {
        private:
            sf::Texture _texture;
            std::vector<Frame> _frames;
            double _delay = 0.0;

        public:
            Animation(std::vector<uint8_t> data, rtype::AssetsPak &assets);

            const sf::Texture &texture() const;
            const Frame &frameAt(double elapsed, bool loop, bool reversed) const;
    };
}
