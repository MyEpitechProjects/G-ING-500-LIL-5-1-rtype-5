#include "Graphic.hpp"
#include "objects/PlayerObject.hpp"
#include "objects/ShootLoadingObject.hpp"
#include "objects/NetworkObject.hpp"

#include <algorithm>

rtype::Graphic::Graphic(uint16_t width, uint16_t height, std::string title, std::string assets_file) : assets(assets_file), object_manager(assets) {
    window.create(sf::VideoMode({width, height}), title);
    window.setVerticalSyncEnabled(true);
    std::unique_ptr<rtype::Object> obj = std::make_unique<rtype::NetworkObject>(object_manager);

    object_manager.add_object("network", obj);
}

rtype::Graphic::~Graphic() {
}

void rtype::Graphic::handle_objects(rtype::InputKeys keys) {
    auto& objects = object_manager.get_object_map();
    std::vector<rtype::Object*> sortedObjects;
    sortedObjects.reserve(objects.size());

    for (auto& [name, value] : objects) {
        sortedObjects.push_back(value.get());
    }
    std::sort(sortedObjects.begin(), sortedObjects.end(),
        [](const rtype::Object* a, const rtype::Object* b) {
            return a->get_layer() < b->get_layer();
        }
    );
    for (auto* obj : sortedObjects) {
        obj->update(keys);
        if (!obj->is_hided())
            obj->render(window);
    }
}

bool rtype::Graphic::is_connected() {
    rtype::NetworkObject* obj = (rtype::NetworkObject*)object_manager.get_object("network");

    if (!obj){
        return false;
    }
    return obj->is_connected() || !obj->has_started();
}


void rtype::Graphic::run() {
    rtype::InputKeys keys{};

    while (window.isOpen()) {
        keys = {};
        if (!is_connected()){
            object_manager.remove_all();
            window.close();
            return;
        }
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()){
                object_manager.remove_all();
                window.close();
            } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {                    
                if (key->code == sf::Keyboard::Key::Escape){
                    object_manager.remove_all();
                    window.close();
                }
            }
        }
        keys.escape = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
        keys.up     = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
        keys.down   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
        keys.left   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
        keys.right  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
        keys.shoot  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
        keys.space  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
        if (keys.escape){
            object_manager.remove_all();
            window.close();
        }
        window.clear(sf::Color(30, 30, 30));
        handle_objects(keys);
        window.display();
    }
}