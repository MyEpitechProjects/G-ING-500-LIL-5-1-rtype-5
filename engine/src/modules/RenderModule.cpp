/*
** EPITECH PROJECT, 2026
** RenderModule
** File description:
** RenderModule
*/

#include "RenderModule.hpp"
#include "Zipper.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

static sf::Color toSfColor(const Color &color) {
    return sf::Color(color.red, color.green, color.blue, color.alpha);
}

static sf::Vector2f toPixel(const Position &position) {
    return {std::round(position.x), std::round(position.y)};
}

RenderModule::RenderModule(unsigned int width, unsigned int height, const std::string &title)
    : AModule("render"), _window(sf::VideoMode({width, height}), title), _clearColor(sf::Color::Black) {}

Position RenderModule::getMousePosition() const {
    sf::Vector2f mouse = _window.mapPixelToCoords(sf::Mouse::getPosition(_window));

    return Position{mouse.x, mouse.y};
}

float RenderModule::getLineSpacing(const std::string &font, unsigned int size) const {
    std::unordered_map<std::string, Font>::const_iterator found = _fonts.find(font);

    if (found == _fonts.end())
        throw RenderException("RenderModule: unknown font " + font);
    return found->second.font.getLineSpacing(size);
}

void RenderModule::loadTexture(const std::string &name, const std::string &path) {
    sf::Texture texture;

    if (!texture.loadFromFile(path))
        throw RenderException("RenderModule: cannot load texture " + path);
    _textures.insert_or_assign(name, std::move(texture));
}

void RenderModule::loadTexture(const std::string &name, const std::vector<std::uint8_t> &data) {
    sf::Texture texture;

    if (data.empty() || !texture.loadFromMemory(data.data(), data.size()))
        throw RenderException("RenderModule: cannot load texture " + name);
    _textures.insert_or_assign(name, std::move(texture));
}

void RenderModule::loadFont(const std::string &name, const std::string &path) {
    Font &font = _fonts[name];

    font.data.clear();
    if (!font.font.openFromFile(path)) {
        _fonts.erase(name);
        throw RenderException("RenderModule: cannot load font " + path);
    }
}

void RenderModule::loadFont(const std::string &name, const std::vector<std::uint8_t> &data) {
    Font &font = _fonts[name];

    font.data = data;
    if (font.data.empty() || !font.font.openFromMemory(font.data.data(), font.data.size())) {
        _fonts.erase(name);
        throw RenderException("RenderModule: cannot load font " + name);
    }
}

void RenderModule::loadAnimation(const std::string &name, const std::string &texture, const std::vector<Frame> &frames, double delay) {
    if (frames.empty())
        throw RenderException("RenderModule: animation " + name + " has no frame");
    _animations.insert_or_assign(name, Animation{texture, frames, delay});
}

void RenderModule::registerComponents(Registry &registry) {
    registry.registerComponent<Position>();
    registry.registerComponent<Sprite>();
    registry.registerComponent<SpriteAnimation>();
    registry.registerComponent<Shape>();
    registry.registerComponent<Text>();
}

void RenderModule::update(Registry &registry, double elapsedSeconds) {
    SparseArray<Position> &positions = registry.getComponents<Position>();
    SparseArray<Sprite> &sprites = registry.getComponents<Sprite>();
    SparseArray<SpriteAnimation> &animations = registry.getComponents<SpriteAnimation>();
    SparseArray<Shape> &shapes = registry.getComponents<Shape>();
    SparseArray<Text> &texts = registry.getComponents<Text>();
    std::vector<Drawable> drawables;

    if (!_window.isOpen())
        return;
    while (const std::optional<sf::Event> event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window.close();
        if (_onEvent)
            _onEvent(*event);
    }
    if (!_window.isOpen())
        return;
    for (std::optional<SpriteAnimation> &animation : animations)
        if (animation)
            animation->time += elapsedSeconds;
    for (std::tuple<Position &, Sprite &> components : Containers::Zipper(positions, sprites))
        if (std::get<1>(components).visible)
            drawables.push_back({std::get<1>(components).layer, &std::get<0>(components), &std::get<1>(components), nullptr, nullptr, nullptr});
    for (std::tuple<Position &, SpriteAnimation &> components : Containers::Zipper(positions, animations))
        if (std::get<1>(components).visible)
            drawables.push_back({std::get<1>(components).layer, &std::get<0>(components), nullptr, &std::get<1>(components), nullptr, nullptr});
    for (std::tuple<Position &, Shape &> components : Containers::Zipper(positions, shapes))
        if (std::get<1>(components).visible)
            drawables.push_back({std::get<1>(components).layer, &std::get<0>(components), nullptr, nullptr, &std::get<1>(components), nullptr});
    for (std::tuple<Position &, Text &> components : Containers::Zipper(positions, texts))
        if (std::get<1>(components).visible)
            drawables.push_back({std::get<1>(components).layer, &std::get<0>(components), nullptr, nullptr, nullptr, &std::get<1>(components)});
    std::sort(drawables.begin(), drawables.end(),
        [](const Drawable &left, const Drawable &right) {
            return left.layer < right.layer;
        }
    );
    _window.clear(_clearColor);
    for (const Drawable &drawable : drawables) {
        if (drawable.sprite)
            draw(*drawable.position, *drawable.sprite);
        else if (drawable.animation)
            draw(*drawable.position, *drawable.animation);
        else if (drawable.shape)
            draw(*drawable.position, *drawable.shape);
        else
            draw(*drawable.position, *drawable.text);
    }
    _window.display();
}

void RenderModule::drawTexture(const Position &position, const std::string &texture, const Frame &frame, bool centered) {
    if (_textures.find(texture) == _textures.end())
        return;
    sf::Sprite drawable(_textures.find(texture)->second, sf::IntRect({frame.left, frame.top}, {frame.width, frame.height}));

    if (centered)
        drawable.setOrigin({frame.width / 2.0f, frame.height / 2.0f});
    drawable.setPosition(toPixel(position));
    _window.draw(drawable);
}

void RenderModule::draw(const Position &position, const Sprite &sprite) {
    drawTexture(position, sprite.texture, Frame{sprite.left, sprite.top, sprite.width, sprite.height}, sprite.centered);
}

void RenderModule::draw(const Position &position, const SpriteAnimation &animation) {
    std::unordered_map<std::string, Animation>::const_iterator found = _animations.find(animation.animation);
    std::size_t index = 0;

    if (found == _animations.end())
        return;
    const std::vector<Frame> &frames = found->second.frames;

    if (found->second.delay > 0.0 && animation.time > 0.0) {
        std::size_t step = static_cast<std::size_t>(animation.time / found->second.delay);

        index = animation.loop ? step % frames.size() : std::min(step, frames.size() - 1);
    }
    if (animation.reversed)
        index = frames.size() - 1 - index;
    drawTexture(position, found->second.texture, frames[index], animation.centered);
}

void RenderModule::draw(const Position &position, const Shape &shape) {
    if (shape.type == ShapeType::Circle) {
        sf::CircleShape circle(shape.width / 2.0f);

        if (shape.centered)
            circle.setOrigin({shape.width / 2.0f, shape.width / 2.0f});
        circle.setPosition({position.x, position.y});
        circle.setFillColor(toSfColor(shape.fill));
        circle.setOutlineColor(toSfColor(shape.outline));
        circle.setOutlineThickness(shape.outlineThickness);
        _window.draw(circle);
        return;
    }
    sf::RectangleShape rectangle({shape.width, shape.height});

    if (shape.centered)
        rectangle.setOrigin({shape.width / 2.0f, shape.height / 2.0f});
    rectangle.setPosition({position.x, position.y});
    rectangle.setFillColor(toSfColor(shape.fill));
    rectangle.setOutlineColor(toSfColor(shape.outline));
    rectangle.setOutlineThickness(shape.outlineThickness);
    _window.draw(rectangle);
}

void RenderModule::draw(const Position &position, const Text &text) {
    if (_fonts.find(text.font) == _fonts.end())
        return;
    sf::Text drawable(_fonts.find(text.font)->second.font, sf::String::fromUtf8(text.content.begin(), text.content.end()), text.size);

    if (text.centered) {
        sf::FloatRect bounds = drawable.getLocalBounds();

        drawable.setOrigin({bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f});
    }
    drawable.setFillColor(toSfColor(text.color));
    drawable.setPosition(toPixel(position));
    _window.draw(drawable);
}
