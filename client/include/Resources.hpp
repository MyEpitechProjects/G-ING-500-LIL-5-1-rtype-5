#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Animation.hpp"
#include "AssetsPak.hpp"

namespace rtype {
    class Resources {
        private:
            rtype::AssetsPak &_pak;
            std::map<std::string, std::shared_ptr<const rtype::Animation>> _animations;
            std::vector<uint8_t> _fontData;
            sf::Font _font;

        public:
            explicit Resources(rtype::AssetsPak &pak);

            std::shared_ptr<const rtype::Animation> animation(const std::string &path);
            const sf::Font &font() const;
    };
}
