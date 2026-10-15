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
            asio::ip::udp::endpoint server;
            std::thread networkThread;
            uint32_t session_id = 0;
            uint32_t sequence = 0;
        public:
            explicit NetworkObject(rtype::ObjectManager& manager_obj);
            ~NetworkObject() override;
            void update(rtype::InputKeys& keys) override;
            void send_input(rtype::InputKeys& keys);
    };
}
