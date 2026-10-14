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
            
        public:
            explicit PlayerObject(uint16_t x, uint16_t y, uint16_t dir, rtype::ObjectManager& manager_obj);
            ~PlayerObject() = default;
            void update(rtype::InputKeys& keys) override;

    };
}
