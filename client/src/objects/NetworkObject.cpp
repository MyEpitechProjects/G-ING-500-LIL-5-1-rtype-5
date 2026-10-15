#include "objects/NetworkObject.hpp"
#include "objects/PlayerObject.hpp"
#include "objects/EnemyObject.hpp"
#include "objects/ShootLoadingObject.hpp"

rtype::NetworkObject::NetworkObject(rtype::ObjectManager& manager_obj) : Object(manager_obj), wake(std::make_shared<WakeSignal>()), network(0, wake), server(asio::ip::make_address("127.0.0.1"), 4242) {
    networkThread = std::thread([this]() {
        try {
            network.run();
        } catch (const std::exception& error) {
            std::cerr << "Erreur du thread réseau : "
                      << error.what() << '\n';
        }
    });
    network.send(
        std::make_unique<Net::HelloPacket>(),
        server
    );
}

void rtype::NetworkObject::send_input(rtype::InputKeys& keys) {
    auto packet = std::make_unique<Net::InputPacket>();
    packet->header.sessionId = session_id;
    packet->header.sequence = 0;
    packet->buttons = 0;
    if (keys.up)
        packet->buttons |= Net::InputButton::UP;
    if (keys.down)
        packet->buttons |= Net::InputButton::DOWN;
    if (keys.left)
        packet->buttons |= Net::InputButton::LEFT;
    if (keys.right)
        packet->buttons |= Net::InputButton::RIGHT;
    network.send(std::move(packet), server);
}

void rtype::NetworkObject::update(rtype::InputKeys& keys) {
    (void)keys;
    while (auto incoming = network.popPacket()) {
        auto message = std::move(*incoming);
        auto& packet = message.packet;
        if (!packet)
            continue;
        if (auto* snapshot = dynamic_cast<Net::SnapshotPacket*>(packet.get())) {
            for (const auto& entity : snapshot->entities) {
                if (entity.kind == Net::EntityKind::PLAYER){
                    std::cout << "PLAYER" << std::endl;
                }
                PlayerObject *player_obj = (PlayerObject *)manager.get_object("player");
                Object *obj = nullptr;
                if (player_obj && player_obj->get_entity_id() == entity.id){
                    obj = (Object *)obj;
                } else {
                    obj = manager.get_object(std::to_string(entity.id));
                }
                if (obj) {
                    obj->set_x(entity.x);
                    obj->set_y(entity.y);
                } else {
                    switch (entity.kind)
                    {
                        case Net::EntityKind::ENEMY: {
                            std::unique_ptr<rtype::Object> obj = std::make_unique<rtype::EnemyObject>(entity.x, entity.y, 0, manager);

                            rtype::Object *ptr = obj.get();
                            ptr->add_animation("blue-stand", manager.get_assets(), "images/animations/ship/blue/stand.anim");
                            ptr->set_animation("blue-stand");
                            manager.add_object(std::to_string(entity.id), obj);
                            break;
                        }
                        case Net::EntityKind::PLAYER:{
                            std::unique_ptr<rtype::Object> obj = std::make_unique<rtype::EnemyObject>(entity.x, entity.y, 0, manager);
                            rtype::Object *ptr = obj.get();
                            ptr->add_animation("blue-stand", manager.get_assets(), "images/animations/ship/blue/stand.anim");
                            ptr->set_animation("blue-stand");
                            manager.add_object(std::to_string(entity.id), obj);
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        } else if (auto* welcome = dynamic_cast<Net::WelcomePacket*>(packet.get())) {
            session_id = welcome->header.sessionId;
            std::cout << std::to_string(session_id) << std::endl;
            std::unique_ptr<rtype::Object>  obj = std::make_unique<rtype::PlayerObject>(0, 0, 0, manager);

            rtype::Object *ptr = obj.get();
            ptr->add_animation("blue-stand", manager.get_assets(), "images/animations/ship/blue/stand.anim");
            ptr->add_animation("blue-down", manager.get_assets(), "images/animations/ship/blue/down.anim");
            ptr->add_animation("blue-up", manager.get_assets(), "images/animations/ship/blue/up.anim");
            ptr->set_animation("blue-stand");
            ptr->set_x(200);
            ptr->set_y(200);
            ((PlayerObject*)ptr)->set_entity_id(welcome->playerEntityId);
            manager.add_object("player", obj);

            



            obj = std::make_unique<rtype::ShootLoadingObject>(0, 0, 0, manager);

            ptr = obj.get();
            ptr->add_animation("anim", manager.get_assets(), "images/animations/shoot_load.anim");
            ptr->set_animation("anim");
            rtype::Animation *anim = ptr->get_animation("anim");
            anim->set_loop(true);
            manager.add_object("gun", obj);
        }
    }
}

rtype::NetworkObject::~NetworkObject() {
    network.stop();

    if (networkThread.joinable())
        networkThread.join();
}