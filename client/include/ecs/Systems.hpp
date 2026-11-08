#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "ClientNet.hpp"
#include "Engine.hpp"
#include "Resources.hpp"
#include "ecs/Components.hpp"

namespace rtype::ecs {

    struct UiState {
        std::optional<Entity> focus;
        sf::Vector2f mouse{};
        float time = 0.0f;
    };

    struct FrameInput {
        std::string typed;
        bool backspace = false;
        bool tab = false;
        bool openChat = false;
        bool escape = false;
        bool clicked = false;
        float wheel = 0.0f;
        sf::Vector2f mouse{};
    };

    // net sync
    class NetSyncSystem {
        public:
            explicit NetSyncSystem(rtype::Resources &resources);

            void reset();
            void operator()(Registry &registry, rtype::ClientNet &net);
            // Temps serveur auquel afficher les entités distantes.
            double renderTime(double localNow) const;
            bool localPlayerDead() const { return _localDead; }
            std::uint16_t localHealth() const { return _localHealth; }
            unsigned int score() const { return _score; }

        private:
            rtype::Resources &_resources;
            interp::ServerClock _clock;
            std::unordered_map<std::uint32_t, Entity> _entities;
            std::unordered_set<std::uint32_t> _destroyed;   // les ids ne sont jamais réutilisés
            std::optional<std::uint32_t> _myEntity;
            std::optional<Entity> _charge;
            std::optional<std::uint32_t> _lastTick;
            bool _localDead = false;
            std::uint16_t _localHealth = 3;
            unsigned int _score = 0;

            Entity spawn(Registry &registry, const Net::EntityState &state);
            void promoteToLocal(Registry &registry, Entity entity);
            void applySnapshot(Registry &registry, const rtype::ClientNet::Snapshot &snapshot);
            void applyEvent(Registry &registry, const rtype::ClientNet::Event &event);
    };

    // gameplay

    class InterpolationSystem {
        public:
            void operator()(Registry &registry, double renderTime) const;
    };

    class PredictionSystem {
        public:
            void operator()(Registry &registry, const Keys &keys, float dt) const;
    };

    class ShipAnimationSystem {
        public:
            void operator()(Registry &registry) const;
    };

    class ChargeEffectSystem {
        public:
            void operator()(Registry &registry, const Keys &keys, float dt) const;
    };

    class AnimationSystem {
        public:
            void operator()(Registry &registry, float dt) const;
    };

    class RenderSystem {
        public:
            void operator()(Registry &registry, sf::RenderWindow &window) const;
    };


    // ui

    class UiInputSystem {
        public:
            void operator()(Registry &registry, UiState &ui, const FrameInput &input) const;
    };

    class UiRenderSystem {
        public:
            void operator()(Registry &registry, sf::RenderWindow &window, const sf::Font &font, const UiState &ui) const;
    };
}
