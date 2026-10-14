#pragma once
#include <map>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <optional>
#include <vector>
#include <cstring>
#include <openssl/sha.h>

namespace fs = std::filesystem;

namespace rtype {
    struct PakHeader {
        uint32_t data_offset;
        uint32_t num_files;

    };
    struct AssetsEntry {
        uint32_t offset;
        uint32_t size;
        uint64_t name;
    };

    class AssetsPak {
        private:
            std::ifstream file;
            std::map<uint64_t, AssetsEntry> value;
            bool status = false;
            uint32_t nb_assets = 0;
            uint32_t base_offset = 0;
            
        public:
            AssetsPak(const fs::path& chemin);
            ~AssetsPak() = default;

            bool is_valid() const;
            std::optional<std::vector<uint8_t>> get_asset(const std::string& path);
    };
}
