#include "Animation.hpp"
#include <cstring>
#include <stdexcept>
#include <string>

rtype::Animation::Animation(std::vector<uint8_t> data, rtype::AssetsPak &assets) {
    size_t offset = 0;
    uint8_t base_texture_l = 0;
    int64_t delay = 0;

    if (data.size() < sizeof(delay))
        throw std::runtime_error("Failed to load animation (step 1/7)");
    std::memcpy(&delay, data.data() + offset, sizeof(delay));
    offset += sizeof(delay);
    if (data.size() - offset < sizeof(base_texture_l))
        throw std::runtime_error("Failed to load animation (step 2/7)");
    std::memcpy(&base_texture_l, data.data() + offset, sizeof(base_texture_l));
    offset += sizeof(base_texture_l);
    if (data.size() - offset < base_texture_l)
        throw std::runtime_error("Failed to load animation (step 3/7)");
    std::string base_texture(reinterpret_cast<const char *>(data.data() + offset), base_texture_l);
    offset += base_texture_l;

    auto texture_data = assets.get_asset(base_texture);
    if (!texture_data || texture_data->empty())
        throw std::runtime_error("Failed to load animation (step 4/7)");
    if (!_texture.loadFromMemory(texture_data->data(), texture_data->size()))
        throw std::runtime_error("Failed to load animation (step 5/7)");

    uint32_t nb_frames = 0;
    if (data.size() - offset < sizeof(nb_frames))
        throw std::runtime_error("Failed to load animation (step 6/7)");
    std::memcpy(&nb_frames, data.data() + offset, sizeof(nb_frames));
    offset += sizeof(nb_frames);
    constexpr size_t frame_size = sizeof(uint16_t) * 4;
    if (nb_frames > (data.size() - offset) / frame_size)
        throw std::runtime_error("Failed to load animation (step 7/7)");
    _frames.reserve(nb_frames);
    for (uint32_t i = 0; i < nb_frames; i++) {
        rtype::Frame frame{};
        std::memcpy(&frame.x, data.data() + offset, sizeof(frame.x));
        offset += sizeof(frame.x);
        std::memcpy(&frame.y, data.data() + offset, sizeof(frame.y));
        offset += sizeof(frame.y);
        std::memcpy(&frame.w, data.data() + offset, sizeof(frame.w));
        offset += sizeof(frame.w);
        std::memcpy(&frame.h, data.data() + offset, sizeof(frame.h));
        offset += sizeof(frame.h);
        _frames.push_back(frame);
    }
    if (_frames.empty())
        throw std::runtime_error("No frames found");
    _delay = static_cast<double>(delay) / 1000.0;
}

const sf::Texture &rtype::Animation::texture() const {
    return _texture;
}

const rtype::Frame &rtype::Animation::frameAt(double elapsed, bool loop, bool reversed) const {
    const std::size_t count = _frames.size();
    std::size_t index = 0;

    if (_delay > 0.0 && elapsed > 0.0) {
        const std::size_t step = static_cast<std::size_t>(elapsed / _delay);
        index = loop ? step % count : std::min(step, count - 1);
    }
    return _frames[reversed ? count - 1 - index : index];
}
