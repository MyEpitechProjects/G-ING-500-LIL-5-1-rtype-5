#include "Graphic.hpp"

rtype::Graphic::Graphic(uint16_t width, uint16_t height, std::string title) {
    window.create(sf::VideoMode({width, height}), title);
}

rtype::Graphic::~Graphic() {
}

void rtype::Graphic::run() {
    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
            }
        }
        window.clear(sf::Color(30, 30, 30));
        window.display();
    }
}