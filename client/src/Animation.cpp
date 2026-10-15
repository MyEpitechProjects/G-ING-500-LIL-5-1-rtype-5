#include "Animation.hpp"
#include <cstring>

rtype::Animation::Animation(std::vector<uint8_t> data, rtype::AssetsPak& assets) {
    size_t offset = 0;

    uint8_t base_texture_l = 0;
    int64_t delay = 0;

    if (data.size() < sizeof(delay))
        throw std::runtime_error("Failed to load 0");

    std::memcpy(&delay, data.data() + offset, sizeof(delay));
    offset += sizeof(delay);

    if (data.size() - offset < sizeof(base_texture_l))
        throw std::runtime_error("Failed to load 1");

    std::memcpy(&base_texture_l, data.data() + offset, sizeof(base_texture_l));
    offset += sizeof(base_texture_l);

    if (data.size() - offset < base_texture_l)
        throw std::runtime_error("Failed to load 2");

    std::string base_texture(
        reinterpret_cast<const char*>(data.data() + offset),
        base_texture_l
    );
    offset += base_texture_l;

    auto texture_data = assets.get_asset(base_texture);

    if (!texture_data || texture_data->empty())
        throw std::runtime_error("Failed to load 3");

    if (!texture.loadFromMemory(texture_data->data(), texture_data->size()))
        throw std::runtime_error("Failed to load 4");

    uint32_t nb_frames = 0;

    if (data.size() - offset < sizeof(nb_frames))
        throw std::runtime_error("Failed to load 5");

    std::memcpy(&nb_frames, data.data() + offset, sizeof(nb_frames));
    offset += sizeof(nb_frames);

    constexpr size_t frame_size = sizeof(uint16_t) * 4;

    if (nb_frames > (data.size() - offset) / frame_size)
        throw std::runtime_error("Failed to load 6");

    frames.reserve(nb_frames);

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

        frames.push_back(frame);
    }
    size = frames.size();
    if (size == 0){
        throw std::runtime_error("No frames");
    }
    animationDelay = sf::milliseconds(delay);
}

const sf::Texture& rtype::Animation::get_texture() const { 
    return texture; 
}

const rtype::Frame& rtype::Animation::next_frame() {
    int64_t elapsed = animationClock.getElapsedTime().asMicroseconds();
    int64_t delayTime = animationDelay.asMicroseconds();

    if (elapsed < (delayTime * size)){
        return get_frame(elapsed / delayTime);
    } else if (loop) {
        animationClock.restart();
        return get_frame(0);
    }
    return get_frame(size - 1);
}

const rtype::Frame& rtype::Animation::get_frame(uint32_t frame_index) const {
    uint32_t index = frame_index % size;
    if (!reversed){
        return frames[index]; 
    }
    return frames[(size - 1) - index]; 
}

void rtype::Animation::reset_animation() {
    animationClock.restart();
}

void rtype::Animation::set_reverse(bool status) {
    reversed = status;
}

void rtype::Animation::set_loop(bool status) {
    loop = status;
}

rtype::Animation::~Animation() {
}
