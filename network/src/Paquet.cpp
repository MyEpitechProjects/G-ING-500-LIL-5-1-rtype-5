#include "Paquet.hpp"
#include "PacketCodec.hpp"
#include <cstring>

std::vector<std::uint8_t> Net::InputPacket::serialize() const {
    if ((buttons & BUTTON_MASK) != buttons)
        throw std::invalid_argument("Invalid input buttons");
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE + 2);
    PacketCodec::writeHeader(out, header, PacketType::INPUT);
    PacketCodec::writeU16(out, buttons);
    return out;
}

std::vector<std::uint8_t> Net::SnapshotPacket::serialize() const {
    if (entities.size() > MAX_SNAPSHOT_ENTITIES)
        throw std::invalid_argument("Too many snapshot entities");
    std::vector<std::uint8_t> out;
    out.reserve(SNAPSHOT_BASE_WIRE_SIZE + entities.size() * ENTITY_WIRE_SIZE);
    PacketCodec::writeHeader(out, header, PacketType::SNAPSHOT);
    PacketCodec::writeU32(out, serverTick);
    PacketCodec::writeU16(out, static_cast<std::uint16_t>(entities.size()));
    for (const auto& entity : entities) {
        if (PacketCodec::validEntity(entity)) {
            PacketCodec::writeU32(out, entity.id);
            out.push_back(static_cast<std::uint8_t>(entity.kind));
            std::uint32_t xBits;
            std::uint32_t yBits;
            std::memcpy(&xBits, &entity.x, sizeof(xBits));
            std::memcpy(&yBits, &entity.y, sizeof(yBits));
            PacketCodec::writeU32(out, xBits);
            PacketCodec::writeU32(out, yBits);
            PacketCodec::writeU16(out, entity.health);
            out.push_back(entity.flags);
        } else {
            throw std::invalid_argument("invalid entity");
        }
    }
    return out;
}

std::vector<std::uint8_t> Net::HelloPacket::serialize() const {
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE);
    PacketCodec::writeHeader(out, header, PacketType::HELLO);
    return out;
}

std::vector<std::uint8_t> Net::WelcomePacket::serialize() const {
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE + 8);
    PacketCodec::writeHeader(out, header, PacketType::WELCOME);
    PacketCodec::writeU32(out, playerEntityId);
    PacketCodec::writeU32(out, entityId);
    return out;
}

std::vector<std::uint8_t> Net::GameEventPacket::serialize() const {
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE + 9);
    PacketCodec::writeHeader(out, header, PacketType::EVENT);
    out.push_back(static_cast<std::uint8_t>(eventType));
    PacketCodec::writeU32(out, entityId);
    PacketCodec::writeU32(out, eventId);
    return out;
}
