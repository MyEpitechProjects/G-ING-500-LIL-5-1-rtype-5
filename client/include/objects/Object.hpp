#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include "AssetsPak.hpp"
#include "Animation.hpp"
#include "ObjectManager.hpp"

namespace rtype {
    class ObjectManager;

    struct InputKeys {
        uint8_t left : 1;
        uint8_t up : 1;
        uint8_t right : 1;
        uint8_t down : 1;
        uint8_t shoot : 1;
        uint8_t escape : 1;
        uint8_t space : 1;
        uint8_t reserved : 1;
        bool operator==(const InputKeys& other) const {
            return left == other.left
                && up == other.up
                && right == other.right
                && down == other.down
                && shoot == other.shoot
                && escape == other.escape
                && space == other.space
                && reserved == other.reserved;
        }
        bool operator!=(const InputKeys& other) const {
            return !(*this == other);
        }
    };

    class Object {
        protected:
            sf::Texture blank_texture;
            sf::Clock objectClock;
            uint16_t layer = 0;
            float x = 0;
            float y = 0;
            float speed = 200.0f;
            uint16_t dir = 0;
            std::map<std::string,std::unique_ptr<rtype::Animation>> animations;
            sf::Sprite sprite;
            std::string animation_id = "";
            bool hided = false;
            rtype::ObjectManager& manager;
            
        public:
            Object(float x, float y, uint16_t dir, rtype::ObjectManager& manager_obj);
            Object(rtype::ObjectManager& manager_obj);
            virtual ~Object();
            const sf::Sprite& get_sprite() const;
            rtype::Animation* get_animation(std::string id);
            virtual void update(rtype::InputKeys& keys) = 0;
            void render(sf::RenderWindow& window);
            uint16_t get_layer() const;
            void move_x(float value);
            void move_y(float value);
            void set_x(float value);
            void set_y(float value);
            void add_animation(std::string animation_id, rtype::AssetsPak& assets, std::string animationPathh);
            void set_animation(std::string animation_id);
            void reset_animation();
            void reverse_animation(bool status);
            bool is_hided();
            void hide();
            void show();
            float get_x();
            float get_y();
            void set_loop(bool status);
    };
}
