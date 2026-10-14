#include "objects/NetworkObject.hpp"
#include "objects/PlayerObject.hpp"

rtype::NetworkObject::NetworkObject(rtype::ObjectManager& manager_obj) : Object(manager_obj), wake(std::make_shared<WakeSignal>()), network(0, wake)
{
    std::cout << "hello" << std::endl;

    networkThread = std::thread([this]() {
        try {
            network.run();
        } catch (const std::exception& error) {
            std::cerr << "Erreur du thread réseau : "
                      << error.what() << '\n';
        }
    });
    std::cout << "hello2" << std::endl;
    asio::ip::udp::endpoint server(
        asio::ip::make_address("127.0.0.1"),
        4242
    );
    network.send(
        std::make_unique<Net::HelloPacket>(),
        server
    );
}

void rtype::NetworkObject::update(rtype::InputKeys& keys)
{
    (void)keys;
    while (auto incoming = network.popPacket()) {
        auto message = std::move(*incoming);
        auto& packet = message.packet;
        if (!packet)
            continue;
        if (auto* snapshot = dynamic_cast<Net::SnapshotPacket*>(packet.get())) {
            std::cout << "snapshot REQUEST" << std::endl;
            for (const auto& entity : snapshot->entities) {
                (void)entity;
            }
        } else if (auto* welcome = dynamic_cast<Net::WelcomePacket*>(packet.get())) {
            PlayerObject *obj = (PlayerObject *)manager.get_object("player");
            if (obj){
                obj->set_entity_id(welcome->playerEntityId);
            }
        }
    }
}

rtype::NetworkObject::~NetworkObject()
{
    network.stop();

    if (networkThread.joinable())
        networkThread.join();
}