#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <optional>
#include <string>
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
            enum class Scene { Connect, Lobby, Game };

            Options _options;
            sf::RenderWindow _window;
            rtype::AssetsPak _assets;
            rtype::Resources _resources;
            Engine _engine;
            rtype::ClientNet _net;
            ecs::NetSyncSystem _sync;
            ecs::UiState _ui;
            Scene _scene = Scene::Connect;
            ecs::Keys _keys;
            ecs::Keys _sentKeys;
            std::string _formError;

            std::optional<Entity> _addressField;
            std::optional<Entity> _portField;
            std::optional<Entity> _connectButton;
            std::optional<Entity> _statusLabel;
            std::optional<Entity> _gameOverLabel;
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

            void connectClicked();
            void updateConnect(bool enter);
            void updateLobby(const ecs::FrameInput &input);
            void updateGame(float dt);

            ecs::FrameInput pollEvents(bool &enter);
            ecs::Keys readKeys() const;
    };
}
