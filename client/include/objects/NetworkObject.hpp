#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include "AssetsPak.hpp"
#include "Animation.hpp"
#include "Object.hpp"
#include "NetworkHandler.hpp"
 #include "WakeSignal.hpp"


namespace rtype {
    class NetworkObject : public Object {
        private:
            bool is_pressed = false;
            std::shared_ptr<WakeSignal> wake;
            Net::NetworkHandler network;
            std::thread networkThread;
        public:
            explicit NetworkObject(rtype::ObjectManager& manager_obj);
            ~NetworkObject() override;
            void update(rtype::InputKeys& keys) override;

    };
}
