/*
** EPITECH PROJECT, 2026
** RenderModule
** File description:
** RenderModule
*/

#include "RenderModule.hpp"
#include "Position.hpp"
#include "Sprite.hpp"
#include "Zipper.hpp"

#include <algorithm>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

RenderModule::RenderModule(unsigned int width, unsigned int height, const std::string &title)
    : AModule("render"), _window(sf::VideoMode({width, height}), title) {}

void RenderModule::loadTexture(const std::string &name, const std::string &path) {
    sf::Texture texture;

    if (!texture.loadFromFile(path))
        throw RenderException("RenderModule: cannot load texture " + path);
    _textures.insert_or_assign(name, std::move(texture));
}

void RenderModule::registerComponents(Registry &registry) {
    registry.registerComponent<Position>();
    registry.registerComponent<Sprite>();
}

void RenderModule::update(Registry &registry, [[maybe_unused]] double elapsedSeconds) {
    SparseArray<Position> &positions = registry.getComponents<Position>();
    SparseArray<Sprite> &sprites = registry.getComponents<Sprite>();
    std::vector<std::pair<const Position *, const Sprite *>> drawables;

    if (!_window.isOpen())
        return;
    while (const std::optional<sf::Event> event = _window.pollEvent())
        if (event->is<sf::Event::Closed>())
            _window.close();
    if (!_window.isOpen())
        return;
    for (std::tuple<Position &, Sprite &> components : Containers::Zipper(positions, sprites))
        drawables.emplace_back(&std::get<0>(components), &std::get<1>(components));
    std::sort(drawables.begin(), drawables.end(),
        [](const std::pair<const Position *, const Sprite *> &left, const std::pair<const Position *, const Sprite *> &right) {
            return left.second->layer < right.second->layer;
        }
    );
    _window.clear();
    for (const std::pair<const Position *, const Sprite *> &drawable : drawables)
        draw(*drawable.first, *drawable.second);
    _window.display();
}

void RenderModule::draw(const Position &position, const Sprite &sprite) {
    std::unordered_map<std::string, sf::Texture>::const_iterator texture = _textures.find(sprite.texture);

    if (texture == _textures.end())
        return;
    sf::Sprite drawable(texture->second, sf::IntRect({sprite.left, sprite.top}, {sprite.width, sprite.height}));

    drawable.setOrigin({sprite.width / 2.0f, sprite.height / 2.0f});
    drawable.setPosition({position.x, position.y});
    _window.draw(drawable);
}
