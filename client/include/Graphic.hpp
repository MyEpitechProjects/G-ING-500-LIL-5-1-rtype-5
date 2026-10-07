#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

namespace rtype {
    class Graphic {
        private:
            bool init = false;
            sf::RenderWindow window;
            
        public:
            Graphic(uint16_t width, uint16_t height, std::string title);
            ~Graphic();

            void run();
    };
}
