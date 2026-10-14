#include "objects/Object.hpp"

rtype::Object::Object(rtype::ObjectManager& manager_obj) : sprite(blank_texture), manager(manager_obj)
{
}

rtype::Object::Object(float x, float y, uint16_t dir, rtype::ObjectManager& manager_obj) : sprite(blank_texture), manager(manager_obj)
{
    (void)dir;
    sprite.setPosition(sf::Vector2f(
        static_cast<float>(x),
        static_cast<float>(y)
    ));
}

const sf::Sprite& rtype::Object::get_sprite() const { 
    return sprite; 
}

void rtype::Object::add_animation(std::string animation_id, rtype::AssetsPak& assets, std::string animationPath)
{
    std::optional<std::vector<uint8_t>> data = assets.get_asset(animationPath);
    if (!data.has_value()){
        throw std::runtime_error("Animation file not found"); 
    }
    animations[animation_id] = std::make_unique<rtype::Animation>(data.value(), assets);
}

void rtype::Object::reset_animation()
{
    rtype::Animation *animation = get_animation(animation_id);
    if (animation){
        animation->reset_animation();
    }
}

void rtype::Object::reverse_animation(bool status)
{
    rtype::Animation *animation = get_animation(animation_id);
    if (animation){
        animation->set_reverse(status);
    }
}

void rtype::Object::set_animation(std::string id)
{
    rtype::Animation *animation = get_animation(id);

    if (!animation){
        sprite.setTexture(blank_texture, true);
        return;
    }
    animation_id = id;
    sprite.setTexture(animation->get_texture(), true);
}

float rtype::Object::get_x() { 
    return x;
}

float rtype::Object::get_y() { 
    return y;
}

void rtype::Object::move_x(float value) { 
    x += value;
}

void rtype::Object::move_y(float value) { 
    y += value;
}

void rtype::Object::set_x(float value) { 
    x = value;
}

void rtype::Object::set_y(float value) { 
    y = value;
}

uint16_t rtype::Object::get_layer() const { 
    return layer; 
}

bool rtype::Object::is_hided() {
    return hided;
}

void rtype::Object::show() {
    hided = false;
}

void rtype::Object::hide() {
    hided = true;
} 

rtype::Animation* rtype::Object::get_animation(std::string id)
{
    auto it = animations.find(id);

    if (it == animations.end())
        return nullptr;

    return it->second.get();
}

void rtype::Object::render(sf::RenderWindow& window) {
    rtype::Animation *animation = get_animation(animation_id);
    if (animation){
        rtype::Frame frame = animation->next_frame();
        sprite.setTextureRect(sf::IntRect(
            {static_cast<int>(frame.x), static_cast<int>(frame.y)},
            {static_cast<int>(frame.w), static_cast<int>(frame.h)}
        ));
    }
    sprite.setPosition(sf::Vector2f(
        {x, y}
    ));
    window.draw(sprite);
}


rtype::Object::~Object() {
}
