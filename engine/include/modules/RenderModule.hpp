/*
** EPITECH PROJECT, 2026
** RenderModule
** File description:
** RenderModule def
*/

#ifndef RENDERMODULE
    #define RENDERMODULE

    #include "AModule.hpp"
    #include "Color.hpp"
    #include "Input.hpp"
    #include "Position.hpp"
    #include "Registry.hpp"
    #include "Shape.hpp"
    #include "Sprite.hpp"
    #include "SpriteAnimation.hpp"
    #include "Text.hpp"
    #include <SFML/Graphics.hpp>
    #include <cstdint>
    #include <exception>
    #include <functional>
    #include <optional>
    #include <string>
    #include <unordered_map>
    #include <utility>
    #include <vector>

class RenderModule : public AModule {
    public:
        struct Frame {
            int left;
            int top;
            int width;
            int height;
        };

    private:
        struct Animation {
            std::string texture;
            std::vector<Frame> frames;
            double delay;
        };

        struct Font {
            std::vector<std::uint8_t> data;
            sf::Font font;
        };

        sf::RenderWindow _window;
        sf::Color _clearColor;
        std::unordered_map<std::string, sf::Texture> _textures;
        std::unordered_map<std::string, Font> _fonts;
        std::unordered_map<std::string, Animation> _animations;
        std::function<void(const Event &)> _onEvent;

        struct Drawable {
            int layer;
            const Position *position;
            const Sprite *sprite;
            const SpriteAnimation *animation;
            const Shape *shape;
            const Text *text;
        };

        std::optional<Event> convert(const sf::Event &event) const;
        void handleEvents();
        void drawTexture(const Position &position, const std::string &texture, const Frame &frame, bool centered);
        void draw(const Position &position, const Sprite &sprite);
        void draw(const Position &position, const SpriteAnimation &animation);
        void draw(const Position &position, const Shape &shape);
        void draw(const Position &position, const Text &text);

    public:
        RenderModule(unsigned int width, unsigned int height, const std::string &title);
        ~RenderModule() override = default;

        bool isOpen() const { return _window.isOpen(); }
        bool hasFocus() const { return _window.hasFocus(); }
        void close() { _window.close(); }
        void setTitle(const std::string &title) { _window.setTitle(title); }
        void setVerticalSync(bool enabled) { _window.setVerticalSyncEnabled(enabled); }
        void onEvent(std::function<void(const Event &)> callback) { _onEvent = std::move(callback); }
        void setClearColor(const Color &color) { _clearColor = sf::Color(color.red, color.green, color.blue, color.alpha); }

        Position getMousePosition() const;
        float getLineSpacing(const std::string &font, unsigned int size) const;

        void loadTexture(const std::string &name, const std::string &path);
        void loadTexture(const std::string &name, const std::vector<std::uint8_t> &data);
        void loadFont(const std::string &name, const std::string &path);
        void loadFont(const std::string &name, const std::vector<std::uint8_t> &data);
        void loadAnimation(const std::string &name, const std::string &texture, const std::vector<Frame> &frames, double delay);

        void registerComponents(Registry &registry) override;
        void update(Registry &registry, double elapsedSeconds) override;

        class RenderException : public std::exception {
            private:
                std::string _message;
            public:
                RenderException(const std::string &message) : _message(message) {}
                const char *what() const noexcept override { return _message.c_str(); }
        };
};

#endif
