/*
** EPITECH PROJECT, 2026
** RenderModule
** File description:
** RenderModule def
*/

#ifndef RENDERMODULE
    #define RENDERMODULE

    #include "AModule.hpp"
    #include "Position.hpp"
    #include "Registry.hpp"
    #include "Sprite.hpp"
    #include <SFML/Graphics.hpp>
    #include <exception>
    #include <string>
    #include <unordered_map>

class RenderModule : public AModule {
    private:
        sf::RenderWindow _window;
        std::unordered_map<std::string, sf::Texture> _textures;

        void draw(const Position &position, const Sprite &sprite);

    public:
        RenderModule(unsigned int width, unsigned int height, const std::string &title);
        ~RenderModule() override = default;

        bool isOpen() const { return _window.isOpen(); }
        void loadTexture(const std::string &name, const std::string &path);

        void registerComponents(Registry &registry) override;
        void update(Registry &registry, double elapsedSeconds) override;

        class RenderException : public std::exception {
            private:
                std::string _message;
            public:
                Exception(const std::string &message) : _message(message) {}
                const char *what() const noexcept override { return _message.c_str(); }
        };
};

#endif
