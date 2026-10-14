#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include "AssetsPak.hpp"
#include "objects/Object.hpp"
namespace rtype {
    class Object;
    
    class ObjectManager {
        protected:
            std::map<std::string,std::unique_ptr<rtype::Object>> objects;
            rtype::AssetsPak& assets;

        public:
            ObjectManager(rtype::AssetsPak& assetsObj);
            ~ObjectManager() = default;
            void add_object(std::string name, std::unique_ptr<rtype::Object>& obj);
            rtype::Object* get_object(std::string id);
            const std::map<std::string,std::unique_ptr<rtype::Object>>& get_object_map() const;
    };
}
