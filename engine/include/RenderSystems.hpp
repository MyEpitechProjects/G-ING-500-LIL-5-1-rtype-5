/*
** EPITECH PROJECT, 2026
** RenderSystems
** File description:
** RenderSystems def
*/

#ifndef RENDERSYSTEMS
    #define RENDERSYSTEMS

    #include "Position.hpp"
    #include "Registry.hpp"
    #include "Shape.hpp"
    #include "Sprite.hpp"
    #include "SpriteAnimation.hpp"
    #include "Text.hpp"
    #include <SFML/Graphics.hpp>
    #include <cstdint>
    #include <string>
    #include <unordered_map>
    #include <vector>

struct AnimationFrame {
    int left;
    int top;
    int width;
    int height;
};

struct AnimationClip {
    std::string texture;
    std::vector<AnimationFrame> frames;
    double delay;
};

struct FontData {
    std::vector<std::uint8_t> data;
    sf::Font font;
};

struct RenderResources {
    std::unordered_map<std::string, sf::Texture> textures;
    std::unordered_map<std::string, FontData> fonts;
    std::unordered_map<std::string, AnimationClip> animations;
};

class AnimationSystem {
    public:
        void operator()(Registry &registry, double elapsedSeconds) const;
};

class DrawSystem {
    private:
        struct Drawable {
            int layer;
            const Position *position;
            const Sprite *sprite;
            const SpriteAnimation *animation;
            const Shape *shape;
            const Text *text;
        };

        void drawTexture(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const std::string &texture, const AnimationFrame &frame, bool centered) const;
        void draw(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const Sprite &sprite) const;
        void draw(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const SpriteAnimation &animation) const;
        void draw(sf::RenderWindow &window, const Position &position, const Shape &shape) const;
        void draw(sf::RenderWindow &window, const RenderResources &resources, const Position &position, const Text &text) const;

    public:
        void operator()(Registry &registry, sf::RenderWindow &window, const RenderResources &resources) const;
};

#endif
