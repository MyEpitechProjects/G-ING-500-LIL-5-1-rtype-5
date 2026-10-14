#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include "AssetsPak.hpp"
#include "Animation.hpp"
#include "Object.hpp"

namespace rtype {
    class ShootLoadingObject : public Object {
        private:
            bool is_pressed = false;
        public:
            explicit ShootLoadingObject(uint16_t x, uint16_t y, uint16_t dir, rtype::ObjectManager& manager_obj);
            ~ShootLoadingObject() = default;
            void update(rtype::InputKeys& keys) override;

    };
}
