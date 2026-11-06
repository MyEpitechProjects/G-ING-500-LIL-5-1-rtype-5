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
    #include "RenderSystems.hpp"
    #include <SFML/Graphics.hpp>
    #include <cstdint>
    #include <exception>
    #include <functional>
    #include <optional>
    #include <string>
    #include <utility>
    #include <vector>

class RenderModule : public AModule {
    private:
        sf::RenderWindow _window;
        sf::Color _clearColor;
        RenderResources _resources;
        std::function<void(const Event &)> _onEvent;

        std::optional<Event> convert(const sf::Event &event) const;
        void handleEvents();

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
        void loadAnimation(const std::string &name, const std::string &texture, const std::vector<AnimationFrame> &frames, double delay);

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
