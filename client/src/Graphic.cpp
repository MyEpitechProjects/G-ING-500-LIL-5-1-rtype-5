#include "Graphic.hpp"
#include "objects/PlayerObject.hpp"
#include "objects/ShootLoadingObject.hpp"

#include <algorithm>

rtype::Graphic::Graphic(uint16_t width, uint16_t height, std::string title, std::string assets_file) : assets(assets_file), object_manager(assets) {
    window.create(sf::VideoMode({width, height}), title);
    

    std::unique_ptr<rtype::Object> obj = std::make_unique<rtype::PlayerObject>(0, 0, 0, object_manager);

    rtype::Object *ptr = obj.get();
    ptr->add_animation("blue-stand", assets, "images/animations/ship/blue/stand.anim");
    ptr->add_animation("blue-down", assets, "images/animations/ship/blue/down.anim");
    ptr->add_animation("blue-up", assets, "images/animations/ship/blue/up.anim");
    ptr->set_animation("blue-stand");

    object_manager.add_object("player", obj);



    obj = std::make_unique<rtype::ShootLoadingObject>(0, 0, 0, object_manager);

    ptr = obj.get();
    ptr->add_animation("anim", assets, "images/animations/shoot_load.anim");
    ptr->set_animation("anim");
    rtype::Animation *anim = ptr->get_animation("anim");
    anim->set_loop(true);
    object_manager.add_object("gun", obj);
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


void rtype::Graphic::run() {
    rtype::InputKeys keys{};

    while (window.isOpen()) {
        keys = {};
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {                    
                if (key->code == sf::Keyboard::Key::Escape){
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
            window.close();
        }
        window.clear(sf::Color(30, 30, 30));
        handle_objects(keys);
        window.display();
    }
}