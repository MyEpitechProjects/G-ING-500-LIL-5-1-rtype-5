#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "AssetsPak.hpp"
#include "ClientNet.hpp"
#include "Engine.hpp"
#include "Resources.hpp"
#include "ecs/Components.hpp"
#include "ecs/Systems.hpp"

namespace rtype {
    class App {
        public:
            struct Options {
                std::string address = "127.0.0.1";
                std::string port = "4242";
                bool autoConnect = false;
                bool autoCreate = true;
                std::uint32_t autoJoin = 0;
            };

            explicit App(Options options);
            void run();

        private:
            enum class Scene { Connect, Lobby, Game, Controls };
            enum class BindAction { Up, Down, Left, Right, Shoot, Chat };

            struct KeyBindings {
                sf::Keyboard::Key up = sf::Keyboard::Key::Up;
                sf::Keyboard::Key down = sf::Keyboard::Key::Down;
                sf::Keyboard::Key left = sf::Keyboard::Key::Left;
                sf::Keyboard::Key right = sf::Keyboard::Key::Right;
                sf::Keyboard::Key shoot = sf::Keyboard::Key::X;
                sf::Keyboard::Key chat = sf::Keyboard::Key::T;
            };

            Options _options;
            sf::RenderWindow _window;
            rtype::AssetsPak _assets;
            rtype::Resources _resources;
            Engine _engine;
            rtype::ClientNet _net;
            ecs::NetSyncSystem _sync;
            ecs::UiState _ui;
            Scene _scene = Scene::Connect;
            Scene _returnScene = Scene::Connect;
            KeyBindings _bindings;
            std::optional<BindAction> _rebinding;
            ecs::Keys _keys;
            ecs::Keys _sentKeys;
            std::string _formError;

            std::optional<Entity> _addressField;
            std::optional<Entity> _portField;
            std::optional<Entity> _connectButton;
            std::optional<Entity> _statusLabel;
            std::optional<Entity> _gameOverLabel;
            std::optional<Entity> _chatInput;
            std::optional<Entity> _livesLabel;
            std::optional<Entity> _scoreLabel;
            std::optional<Entity> _hudBar;
            int _seenHealth = -1;
            int _seenScore = -1;
            bool _chatOpen = false;
            float _chatPreviewTimer = 0.0f;
            std::vector<rtype::ClientNet::ChatMessage> _chatHistory;
            unsigned _seenRoomsVersion = 0;
            rtype::ClientNet::State _seenState = rtype::ClientNet::State::Idle;
            std::size_t _scroll = 0;
            std::size_t _seenScroll = 0;
            bool _autoPending = false;

            void registerComponents();
            void setScene(Scene scene);
            void clearScene();

            Entity addLabel(float x, float y, const std::string &text, unsigned size, sf::Color color, bool centered);
            Entity addButton(float x, float y, float width, float height, const std::string &label,
                sf::Color fill, std::function<void()> onClick, bool enabled = true);
            Entity addInput(float x, float y, float width, float height, const std::string &value,
                const std::string &placeholder, bool digitsOnly, std::size_t maxLength);
            void setLabel(Entity entity, const std::string &text, sf::Color color);
            std::string fieldValue(const std::optional<Entity> &field);

            void buildConnect();
            void buildLobby();
            void rebuildRooms();
            void buildGame();
            void openControls();
            void buildControls();
            void rebuildControls();
            void applyRebind(sf::Keyboard::Key key);
            sf::Keyboard::Key &bindingFor(BindAction action);
            sf::Keyboard::Key bindingFor(BindAction action) const;
            void setChatOpen(bool open);
            void rebuildChat();
            void submitChat();

            void connectClicked();
            void updateConnect(bool enter);
            void updateLobby(const ecs::FrameInput &input);
            void updateControls();
            void updateGame(float dt, const ecs::FrameInput &input, bool enter);
            void updateHud();

            ecs::FrameInput pollEvents(bool &enter);
            ecs::Keys readKeys() const;
    };
}
