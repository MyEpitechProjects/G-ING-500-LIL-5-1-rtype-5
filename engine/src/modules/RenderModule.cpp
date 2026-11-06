/*
** EPITECH PROJECT, 2026
** RenderModule
** File description:
** RenderModule
*/

#include "RenderModule.hpp"

#include <optional>
#include <utility>
#include <vector>

static Key toKey(sf::Keyboard::Key key) {
    if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z)
        return static_cast<Key>(static_cast<int>(Key::A) + static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::A));
    if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9)
        return static_cast<Key>(static_cast<int>(Key::Num0) + static_cast<int>(key) - static_cast<int>(sf::Keyboard::Key::Num0));
    switch (key) {
        case sf::Keyboard::Key::Left:
            return Key::Left;
        case sf::Keyboard::Key::Right:
            return Key::Right;
        case sf::Keyboard::Key::Up:
            return Key::Up;
        case sf::Keyboard::Key::Down:
            return Key::Down;
        case sf::Keyboard::Key::Space:
            return Key::Space;
        case sf::Keyboard::Key::Enter:
            return Key::Enter;
        case sf::Keyboard::Key::Escape:
            return Key::Escape;
        case sf::Keyboard::Key::Backspace:
            return Key::Backspace;
        case sf::Keyboard::Key::Tab:
            return Key::Tab;
        default:
            return Key::Unknown;
    }
}

static MouseButton toMouseButton(sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Right)
        return MouseButton::Right;
    if (button == sf::Mouse::Button::Middle)
        return MouseButton::Middle;
    return MouseButton::Left;
}

RenderModule::RenderModule(unsigned int width, unsigned int height, const std::string &title)
    : AModule("render"), _window(sf::VideoMode({width, height}), title), _clearColor(sf::Color::Black) {}

Position RenderModule::getMousePosition() const {
    sf::Vector2f mouse = _window.mapPixelToCoords(sf::Mouse::getPosition(_window));

    return Position{mouse.x, mouse.y};
}

float RenderModule::getLineSpacing(const std::string &font, unsigned int size) const {
    std::unordered_map<std::string, FontData>::const_iterator found = _resources.fonts.find(font);

    if (found == _resources.fonts.end())
        throw RenderException("RenderModule: unknown font " + font);
    return found->second.font.getLineSpacing(size);
}

void RenderModule::loadTexture(const std::string &name, const std::string &path) {
    sf::Texture texture;

    if (!texture.loadFromFile(path))
        throw RenderException("RenderModule: cannot load texture " + path);
    _resources.textures.insert_or_assign(name, std::move(texture));
}

void RenderModule::loadTexture(const std::string &name, const std::vector<std::uint8_t> &data) {
    sf::Texture texture;

    if (data.empty() || !texture.loadFromMemory(data.data(), data.size()))
        throw RenderException("RenderModule: cannot load texture " + name);
    _resources.textures.insert_or_assign(name, std::move(texture));
}

void RenderModule::loadFont(const std::string &name, const std::string &path) {
    FontData &font = _resources.fonts[name];

    font.data.clear();
    if (!font.font.openFromFile(path)) {
        _resources.fonts.erase(name);
        throw RenderException("RenderModule: cannot load font " + path);
    }
}

void RenderModule::loadFont(const std::string &name, const std::vector<std::uint8_t> &data) {
    FontData &font = _resources.fonts[name];

    font.data = data;
    if (font.data.empty() || !font.font.openFromMemory(font.data.data(), font.data.size())) {
        _resources.fonts.erase(name);
        throw RenderException("RenderModule: cannot load font " + name);
    }
}

void RenderModule::loadAnimation(const std::string &name, const std::string &texture, const std::vector<AnimationFrame> &frames, double delay) {
    if (frames.empty())
        throw RenderException("RenderModule: animation " + name + " has no frame");
    _resources.animations.insert_or_assign(name, AnimationClip{texture, frames, delay});
}

std::optional<Event> RenderModule::convert(const sf::Event &event) const {
    if (event.is<sf::Event::Closed>())
        return Event{EventType::Closed};
    if (event.is<sf::Event::FocusLost>())
        return Event{EventType::FocusLost};
    if (const sf::Event::KeyPressed *pressed = event.getIf<sf::Event::KeyPressed>())
        return Event{EventType::KeyPressed, toKey(pressed->code)};
    if (const sf::Event::KeyReleased *released = event.getIf<sf::Event::KeyReleased>())
        return Event{EventType::KeyReleased, toKey(released->code)};
    if (const sf::Event::TextEntered *text = event.getIf<sf::Event::TextEntered>())
        return Event{EventType::TextEntered, Key::Unknown, text->unicode};
    if (const sf::Event::MouseButtonPressed *pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mouse = _window.mapPixelToCoords(pressed->position);
        return Event{EventType::MouseButtonPressed, Key::Unknown, 0, toMouseButton(pressed->button), mouse.x, mouse.y};
    }
    if (const sf::Event::MouseButtonReleased *released = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f mouse = _window.mapPixelToCoords(released->position);
        return Event{EventType::MouseButtonReleased, Key::Unknown, 0, toMouseButton(released->button), mouse.x, mouse.y};
    }
    if (const sf::Event::MouseWheelScrolled *wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        sf::Vector2f mouse = _window.mapPixelToCoords(wheel->position);
        return Event{EventType::MouseWheelScrolled, Key::Unknown, 0, MouseButton::Left, mouse.x, mouse.y, wheel->delta};
    }
    return std::nullopt;
}

void RenderModule::handleEvents() {
    while (const std::optional<sf::Event> event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            _window.close();
        std::optional<Event> converted = convert(*event);
        if (converted && _onEvent)
            _onEvent(*converted);
    }
}

void RenderModule::registerComponents(Registry &registry) {
    registry.registerComponent<Position>();
    registry.registerComponent<Sprite>();
    registry.registerComponent<SpriteAnimation>();
    registry.registerComponent<Shape>();
    registry.registerComponent<Text>();
}

void RenderModule::update(Registry &registry, double elapsedSeconds) {
    if (!_window.isOpen())
        return;
    handleEvents();
    if (!_window.isOpen())
        return;
    AnimationSystem{}(registry, elapsedSeconds);
    _window.clear(_clearColor);
    DrawSystem{}(registry, _window, _resources);
    _window.display();
}
