#include "ObjectManager.hpp"

rtype::ObjectManager::ObjectManager(rtype::AssetsPak& assetsObj) : assets(assetsObj) {
}

void rtype::ObjectManager::add_object(std::string name, std::unique_ptr<rtype::Object>& obj) {
    objects[name] = std::move(obj);
}

rtype::Object* rtype::ObjectManager::get_object(std::string id) {
    auto it = objects.find(id);

    if (it == objects.end())
        return nullptr;
    return it->second.get();
}

void rtype::ObjectManager::remove_object(const std::string& id)
{
    auto it = objects.find(id);

    if (it == objects.end())
        return;

    objects.erase(it);
}

const std::map<std::string,std::unique_ptr<rtype::Object>>& rtype::ObjectManager::get_object_map() const {
    return objects;
}

rtype::AssetsPak& rtype::ObjectManager::get_assets() {
    return assets;
}
