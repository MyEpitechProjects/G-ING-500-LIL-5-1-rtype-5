/*
** EPITECH PROJECT, 2026
** RenderModule
** File description:
** RenderModule
*/

#include "RenderModule.hpp"
#include "Position.hpp"
#include "Shape.hpp"
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
    registry.registerComponent<Shape>();
}

void RenderModule::update(Registry &registry, [[maybe_unused]] double elapsedSeconds) {
    SparseArray<Position> &positions = registry.getComponents<Position>();
    SparseArray<Sprite> &sprites = registry.getComponents<Sprite>();
    SparseArray<Shape> &shapes = registry.getComponents<Shape>();
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
    for (std::tuple<Position &, Sprite &> components : Containers::Zipper(positions, sprites))
        drawables.push_back({std::get<1>(components).layer, &std::get<0>(components), &std::get<1>(components), nullptr});
    for (std::tuple<Position &, Shape &> components : Containers::Zipper(positions, shapes))
        drawables.push_back({std::get<1>(components).layer, &std::get<0>(components), nullptr, &std::get<1>(components)});
    std::sort(drawables.begin(), drawables.end(),
        [](const Drawable &left, const Drawable &right) {
            return left.layer < right.layer;
        }
    );
    _window.clear();
    for (const Drawable &drawable : drawables) {
        if (drawable.sprite)
            draw(*drawable.position, *drawable.sprite);
        else
            draw(*drawable.position, *drawable.shape);
    }
    _window.display();
}

void RenderModule::draw(const Position &position, const Sprite &sprite) {
    if (_textures.find(sprite.texture) == _textures.end())
        return;
    sf::Sprite drawable(_textures.find(sprite.texture)->second, sf::IntRect({sprite.left, sprite.top}, {sprite.width, sprite.height}));

    drawable.setOrigin({sprite.width / 2.0f, sprite.height / 2.0f});
    drawable.setPosition({position.x, position.y});
    _window.draw(drawable);
}

void RenderModule::draw(const Position &position, const Shape &shape) {
    sf::Color color(shape.red, shape.green, shape.blue);

    if (shape.type == ShapeType::Circle) {
        sf::CircleShape circle(shape.width / 2.0f);

        circle.setOrigin({shape.width / 2.0f, shape.width / 2.0f});
        circle.setPosition({position.x, position.y});
        circle.setFillColor(color);
        _window.draw(circle);
        return;
    }
    sf::RectangleShape rectangle({shape.width, shape.height});

    rectangle.setOrigin({shape.width / 2.0f, shape.height / 2.0f});
    rectangle.setPosition({position.x, position.y});
    rectangle.setFillColor(color);
    _window.draw(rectangle);
}
