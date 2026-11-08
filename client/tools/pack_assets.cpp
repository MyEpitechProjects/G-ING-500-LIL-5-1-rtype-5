// Asset packer: generates .anim files, then packs a directory into assets.pak.
#include <openssl/sha.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct Frame {
    uint16_t x, y, w, h;
};

using Bytes = std::vector<uint8_t>;

template <typename T>
void put(Bytes& out, T value) {
    for (size_t i = 0; i < sizeof(T); i++)
        out.push_back(static_cast<uint8_t>((static_cast<uint64_t>(value) >> (8 * i)) & 0xFF));
}

void writeFile(const fs::path& path, const Bytes& data) {
    if (path.has_parent_path())
        fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    if (!out)
        throw std::runtime_error("cannot write " + path.string());
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

void addAnimation(const fs::path& base, const std::string& outputPath,
                  const std::string& baseTexture, const std::vector<Frame>& frames, int64_t delay) {
    if (baseTexture.size() >= 256)
        throw std::invalid_argument("Base texture path must be lower than 256");
    Bytes f;
    put<int64_t>(f, delay);
    put<uint8_t>(f, static_cast<uint8_t>(baseTexture.size()));
    f.insert(f.end(), baseTexture.begin(), baseTexture.end());
    put<uint32_t>(f, static_cast<uint32_t>(frames.size()));
    for (const auto& fr : frames) {
        put<uint16_t>(f, fr.x);
        put<uint16_t>(f, fr.y);
        put<uint16_t>(f, fr.w);
        put<uint16_t>(f, fr.h);
    }
    std::cout << (base / outputPath).generic_string() << "\n";
    writeFile(base / outputPath, f);
}

std::vector<Frame> grid2Columns() {
    std::vector<Frame> r;
    for (int i = 0; i < 8; i++)
        r.push_back({static_cast<uint16_t>(i % 2 ? 264 : 0), static_cast<uint16_t>(143 * (i / 2)), 264, 143});
    return r;
}

std::vector<Frame> row(int count, int w, int h, int startX, int startY) {
    std::vector<Frame> r;
    for (int i = 0; i < count; i++)
        r.push_back({static_cast<uint16_t>(startX + w * i), static_cast<uint16_t>(startY),
                     static_cast<uint16_t>(w), static_cast<uint16_t>(h)});
    return r;
}

void packAssets(const fs::path& base, const fs::path& outputPak) {
    addAnimation(base, "images/animations/anim1.anim", "images/sheets/r-typesheet35.gif", grid2Columns(), 100);
    addAnimation(base, "images/animations/ship/blue/stand.anim", "images/sheets/r-typesheet42.gif",
                 {{66, 0, 33, 17}}, 200);
    addAnimation(base, "images/animations/ship/blue/down.anim", "images/sheets/r-typesheet42.gif",
                 {{66, 0, 33, 17}, {33, 0, 33, 17}, {0, 0, 33, 17}}, 200);
    addAnimation(base, "images/animations/ship/blue/up.anim", "images/sheets/r-typesheet42.gif",
                 {{66, 0, 33, 17}, {100, 0, 33, 17}, {133, 0, 33, 17}}, 200);
    addAnimation(base, "images/animations/shoot_load.anim", "images/sheets/r-typesheet1.gif",
                 row(8, 31, 32, 0, 49), 75);
    addAnimation(base, "images/animations/enemy.anim", "images/sheets/r-typesheet2.gif",
                 row(6, 24, 16, 157, 35), 200);
    addAnimation(base, "images/animations/bullet.anim", "images/sheets/r-typesheet1.gif",
                 {{214, 85, 18, 12}, {232, 85, 18, 12}, {250, 85, 18, 12}}, 50);

    std::vector<std::string> files;
    for (const auto& e : fs::recursive_directory_iterator(base))
        if (e.is_regular_file())
            files.push_back(fs::relative(e.path(), base).generic_string());
    std::sort(files.begin(), files.end());

    Bytes f, data;
    put<uint32_t>(f, static_cast<uint32_t>(files.size()));
    uint32_t offset = 0;
    for (const auto& rel : files) {
        std::ifstream in(base / rel, std::ios::binary);
        Bytes content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        data.insert(data.end(), content.begin(), content.end());
        put<uint32_t>(f, offset);
        put<uint32_t>(f, static_cast<uint32_t>(content.size()));
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(rel.data()), rel.size(), hash);
        f.insert(f.end(), hash, hash + 8);
        offset += static_cast<uint32_t>(content.size());
    }

    Bytes out;
    put<uint32_t>(out, static_cast<uint32_t>(f.size() + 4));
    out.insert(out.end(), f.begin(), f.end());
    out.insert(out.end(), data.begin(), data.end());
    writeFile(outputPak, out);
    std::cout << "[Assets Packer] " << files.size() << " files packed in '" << outputPak.string() << "'\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: pack_assets <assets_dir> <output_pak>\n";
        return 1;
    }
    try {
        packAssets(argv[1], argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "pack_assets: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
