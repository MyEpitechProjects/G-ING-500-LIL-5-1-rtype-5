#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "Animation.hpp"
#include "Interpolation.hpp"
#include "Paquet.hpp"

namespace rtype::ecs {

    struct Keys {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        bool shoot = false;

        bool operator==(const Keys &other) const = default;
    };

    inline std::uint16_t toButtons(const Keys &keys) {
        std::uint16_t buttons = 0;
        if (keys.up)
            buttons |= Net::InputButton::UP;
        if (keys.down)
            buttons |= Net::InputButton::DOWN;
        if (keys.left)
            buttons |= Net::InputButton::LEFT;
        if (keys.right)
            buttons |= Net::InputButton::RIGHT;
        if (keys.shoot)
            buttons |= Net::InputButton::FIRE;
        return buttons;
    }

    struct Transform {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Renderable {
        std::shared_ptr<const rtype::Animation> animation;
        double time = 0.0;
        bool loop = false;
        bool reversed = false;
        bool visible = true;
        int layer = 0;
    };

    struct NetId {
        std::uint32_t id = 0;
        Net::EntityKind kind = Net::EntityKind::PLAYER;
    };

    struct Interpolated {
        interp::SnapshotBuffer buffer;
    };

    struct LocalPlayer {
        float idleTime = 0.0f;
    };

    struct ShipAnimation {
        std::shared_ptr<const rtype::Animation> up;
        std::shared_ptr<const rtype::Animation> down;
        float lastY = 0.0f;
        int direction = 0; // -1 up, 1 down
    };

    struct ChargeEffect {
        bool pressed = false;
        float held = 0.0f;
    };

    struct UiBox {
        sf::Vector2f size{100.0f, 40.0f};
        sf::Color fill{40, 40, 50};
        sf::Color outline{90, 90, 110};
    };

    struct UiText {
        std::string text;
        unsigned size = 18;
        sf::Color color = sf::Color::White;
        bool centered = false;
    };

    struct UiInput {
        std::string value;
        std::string placeholder;
        std::size_t maxLength = 32;
        bool digitsOnly = false;
    };

    struct UiButton {
        std::function<void()> onClick;
        bool enabled = true;
    };

    struct UiRow {};

    struct UiChat {};
}
