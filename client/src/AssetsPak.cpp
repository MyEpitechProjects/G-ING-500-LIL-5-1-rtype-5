#include "AssetsPak.hpp"

rtype::AssetsPak::AssetsPak(const fs::path& chemin) {
    status = false;

    if (!fs::exists(chemin))
        return;

    file.open(chemin, std::ios::binary);
    if (!file.is_open())
        return;

    file.read(reinterpret_cast<char*>(&base_offset), sizeof(base_offset));
    file.read(reinterpret_cast<char*>(&nb_assets), sizeof(nb_assets));
    for (uint32_t i = 0; i < nb_assets; i++) {
        AssetsEntry entry{};
        file.read(reinterpret_cast<char*>(&entry), sizeof(entry));
        if (!file) {
            value.clear();
            return;
        }
        value[entry.name] = entry;
    }
    status = true;
}

bool rtype::AssetsPak::is_valid() const {
    return status;
}

std::optional<std::vector<uint8_t>> rtype::AssetsPak::get_asset(const std::string& path) {
    if (!status)
        return std::nullopt;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(path.c_str()), path.size(), hash);
    uint64_t path_hash = 0;
    std::memcpy(&path_hash, hash, sizeof(uint64_t));
    auto it = value.find(path_hash);
    if (it == value.end())
        return std::nullopt;

    const AssetsEntry& entry = it->second;
    file.seekg(base_offset + entry.offset, std::ios::beg);
    std::vector<uint8_t> buffer(entry.size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), entry.size))
        return std::nullopt;
    return buffer;
}