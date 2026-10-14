#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include "AssetsPak.hpp"
#include "Animation.hpp"
#include "Object.hpp"

namespace rtype {
    class PlayerObject : public Object {
        private:
            uint16_t hp = 0;
            bool is_moving_up = false;
            bool is_moving_down = false;
            uint32_t entity_id = 0;
            
        public:
            explicit PlayerObject(uint16_t x, uint16_t y, uint16_t dir, rtype::ObjectManager& manager_obj);
            ~PlayerObject() = default;
            void update(rtype::InputKeys& keys) override;
            uint32_t get_entity_id();
            void set_entity_id(uint32_t id);

    };
}
