#include "Resources.hpp"
#include <stdexcept>

rtype::Resources::Resources(rtype::AssetsPak &pak) : _pak(pak) {
    auto data = _pak.get_asset("fonts/DejaVuSans.ttf");

    if (!data || data->empty())
        throw std::runtime_error("font fonts/DejaVuSans.ttf not found in assets");
    _fontData = std::move(*data);
    if (!_font.openFromMemory(_fontData.data(), _fontData.size()))
        throw std::runtime_error("failed to load font");
}

std::shared_ptr<const rtype::Animation> rtype::Resources::animation(const std::string &path) {
    auto it = _animations.find(path);

    if (it != _animations.end())
        return it->second;
    auto data = _pak.get_asset(path);
    if (!data)
        throw std::runtime_error("animation not found : " + path);
    auto animation = std::make_shared<const rtype::Animation>(std::move(*data), _pak);
    _animations.emplace(path, animation);
    return animation;
}

const sf::Font &rtype::Resources::font() const {
    return _font;
}
