/*
** EPITECH PROJECT, 2026
** DrawSystem
** File description:
** DrawSystem
*/

#include "RenderSystems.hpp"
#include "Zipper.hpp"

#include <algorithm>
#include <cmath>
#include <tuple>

static sf::Color toSfColor(const Color &color) {
    return sf::Color(color.red, color.green, color.blue, color.alpha);
}

static sf::Vector2f toPixel(const Position &position) {
    return {std::round(position.x), std::round(position.y)};
}

void DrawSystem::operator()(Registry &registry, sf::RenderWindow &window, const RenderResources &resources) const {
    SparseArray<Position> &positions = registry.getComponents<Position>();
    SparseArray<Sprite> &sprites = registry.getComponents<Sprite>();
    SparseArray<SpriteAnimation> &animations = registry.getComponents<SpriteAnimation>();
    SparseArray<Shape> &shapes = registry.getComponents<Shape>();
    SparseArray<Text> &texts = registry.getComponents<Text>();
    std::vector<Drawable> drawables;

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
    for (const Drawable &drawable : drawables) {
        if (drawable.sprite)
            draw(window, resources, *drawable.position, *drawable.sprite);
        else if (drawable.animation)
            draw(window, resources, *drawable.position, *drawable.animation);
        else if (drawable.shape)
            draw(window, *drawable.position, *drawable.shape);
        else
            draw(window, resources, *drawable.position, *drawable.text);
    }
}

void DrawSystem::drawTexture(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const std::string &texture, const AnimationFrame &frame, bool centered) const {
    if (resources.textures.find(texture) == resources.textures.end())
        return;
    sf::Sprite drawable(resources.textures.find(texture)->second, sf::IntRect({frame.left, frame.top}, {frame.width, frame.height}));

    if (centered)
        drawable.setOrigin({frame.width / 2.0f, frame.height / 2.0f});
    drawable.setPosition(toPixel(position));
    window.draw(drawable);
}

void DrawSystem::draw(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const Sprite &sprite) const {
    drawTexture(window, resources, position, sprite.texture, AnimationFrame{sprite.left, sprite.top, sprite.width, sprite.height}, sprite.centered);
}

void DrawSystem::draw(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const SpriteAnimation &animation) const {
    std::unordered_map<std::string, AnimationClip>::const_iterator found = resources.animations.find(animation.animation);
    std::size_t index = 0;

    if (found == resources.animations.end())
        return;
    const std::vector<AnimationFrame> &frames = found->second.frames;

    if (found->second.delay > 0.0 && animation.time > 0.0) {
        std::size_t step = static_cast<std::size_t>(animation.time / found->second.delay);

        index = animation.loop ? step % frames.size() : std::min(step, frames.size() - 1);
    }
    if (animation.reversed)
        index = frames.size() - 1 - index;
    drawTexture(window, resources, position, found->second.texture, frames[index], animation.centered);
}

void DrawSystem::draw(sf::RenderWindow &window, const Position &position, const Shape &shape) const {
    if (shape.type == ShapeType::Circle) {
        sf::CircleShape circle(shape.width / 2.0f);

        if (shape.centered)
            circle.setOrigin({shape.width / 2.0f, shape.width / 2.0f});
        circle.setPosition({position.x, position.y});
        circle.setFillColor(toSfColor(shape.fill));
        circle.setOutlineColor(toSfColor(shape.outline));
        circle.setOutlineThickness(shape.outlineThickness);
        window.draw(circle);
        return;
    }
    sf::RectangleShape rectangle({shape.width, shape.height});

    if (shape.centered)
        rectangle.setOrigin({shape.width / 2.0f, shape.height / 2.0f});
    rectangle.setPosition({position.x, position.y});
    rectangle.setFillColor(toSfColor(shape.fill));
    rectangle.setOutlineColor(toSfColor(shape.outline));
    rectangle.setOutlineThickness(shape.outlineThickness);
    window.draw(rectangle);
}

void DrawSystem::draw(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const Text &text) const {
    if (resources.fonts.find(text.font) == resources.fonts.end())
        return;
    sf::Text drawable(resources.fonts.find(text.font)->second.font, sf::String::fromUtf8(text.content.begin(), text.content.end()), text.size);

    if (text.centered) {
        sf::FloatRect bounds = drawable.getLocalBounds();

        drawable.setOrigin({bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f});
    }
    drawable.setFillColor(toSfColor(text.color));
    drawable.setPosition(toPixel(position));
    window.draw(drawable);
}
