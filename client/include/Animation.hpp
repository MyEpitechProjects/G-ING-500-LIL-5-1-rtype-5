#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
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
            sf::Texture texture;
            std::vector<Frame> frames;
            uint32_t size = 0;
            bool loop = false;
            sf::Clock animationClock;
            sf::Time animationDelay = sf::milliseconds(0);
            bool reversed = false;
        public:
            Animation(std::vector<uint8_t> data, rtype::AssetsPak &assets);
            ~Animation();
            const sf::Texture& get_texture() const;
            const rtype::Frame& next_frame();
            void reset_animation();
            const rtype::Frame& get_frame(uint32_t frame_index) const;
            void set_reverse(bool status);
            void set_loop(bool status);

    };
}
