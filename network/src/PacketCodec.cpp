#include "PacketCodec.hpp"

void Net::PacketCodec::writeU16(std::vector<std::uint8_t>& out, std::uint16_t value)
{
    out.push_back(static_cast<std::uint8_t>(value >> 8));
    out.push_back(static_cast<std::uint8_t>(value));
}

void Net::PacketCodec::writeU32(std::vector<std::uint8_t>& out, std::uint32_t value)
{
    writeU16(out, static_cast<std::uint16_t>(value >> 16));
    writeU16(out, static_cast<std::uint16_t>(value));
}

std::uint16_t Net::PacketCodec::readU16(const std::uint8_t* data)
{
    return (std::uint16_t(data[0]) << 8)
         | std::uint16_t(data[1]);
}

std::uint32_t Net::PacketCodec::readU32(const std::uint8_t* data)
{
    return (std::uint32_t(readU16(data)) << 16)
         | std::uint32_t(readU16(data + 2));
}

bool Net::PacketCodec::validEntity(const Net::EntityState& entity) {
    return (entity.kind == Net::EntityKind::PLAYER
        || entity.kind == Net::EntityKind::ENEMY
        || entity.kind == Net::EntityKind::PROJECTILE)
        && std::isfinite(entity.x) && std::isfinite(entity.y);
}

void Net::PacketCodec::writeHeader(std::vector<std::uint8_t>& out,
    const PacketHeader& header, PacketType expected)
{
    if (header.type != expected || header.version != PROTOCOL_VERSION)
        throw std::invalid_argument("Invalid packet header");
    out.push_back(static_cast<std::uint8_t>(header.type));
    out.push_back(header.version);
    writeU16(out, header.sequence);
    writeU32(out, header.sessionId);
}

std::vector<std::uint8_t> Net::PacketCodec::serialize(const InputPacket& packet)
{
    if ((packet.buttons & BUTTON_MASK) != packet.buttons)
        throw std::invalid_argument("Invalid input buttons");
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE + 2);
    writeHeader(out, packet.header, PacketType::INPUT);
    writeU16(out, packet.buttons);
    return out;
}

std::vector<std::uint8_t> Net::PacketCodec::serialize(const HelloPacket& packet)
{
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE);
    writeHeader(out, packet.header, PacketType::HELLO);
    return out;
}

std::vector<std::uint8_t> Net::PacketCodec::serialize(const WelcomePacket& packet)
{
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE + 4);
    writeHeader(out, packet.header, PacketType::WELCOME);
    writeU32(out, packet.playerEntityId);
    return out;
}

std::vector<std::uint8_t> Net::PacketCodec::serialize(const SnapshotPacket& packet)
{
    if (packet.entities.size() > MAX_SNAPSHOT_ENTITIES)
        throw std::invalid_argument("Too many snapshot entities");
    std::vector<std::uint8_t> out;
    out.reserve(SNAPSHOT_BASE_WIRE_SIZE + packet.entities.size() * ENTITY_WIRE_SIZE);
    writeHeader(out, packet.header, PacketType::SNAPSHOT);
    writeU32(out, packet.serverTick);
    writeU16(out, static_cast<std::uint16_t>(packet.entities.size()));
    // Entity wire order: id, kind, x, y, health, flags (16 bytes).
    for (const auto& entity : packet.entities) {
        if (validEntity(entity)) {
            writeU32(out, entity.id);
            out.push_back(static_cast<std::uint8_t>(entity.kind));
            writeU32(out, std::bit_cast<std::uint32_t, float>(entity.x));
            writeU32(out, std::bit_cast<std::uint32_t, float>(entity.y));
            writeU16(out, entity.health);
            out.push_back(entity.flags);
        }
    }
    return out;
}

std::optional<Net::PacketCodec::Packet> Net::PacketCodec::deserialize(const std::uint8_t* data, std::size_t size) {
    if (data == nullptr || size < HEADER_WIRE_SIZE || size > MAX_DATAGRAM_SIZE
        || data[1] != PROTOCOL_VERSION)
        return std::nullopt;
    const PacketHeader header{static_cast<PacketType>(data[0]), data[1],
        readU16(data + 2), readU32(data + 4)};
    switch (header.type) {
        case PacketType::HELLO:
            if (size != HEADER_WIRE_SIZE)
                return std::nullopt;
            return HelloPacket{header};
        case PacketType::WELCOME:
            if (size != HEADER_WIRE_SIZE + 4)
                return std::nullopt;
            return WelcomePacket{header, readU32(data + HEADER_WIRE_SIZE)};
        case PacketType::INPUT: {
            if (size != HEADER_WIRE_SIZE + 2)
                return std::nullopt;
            const auto buttons = readU16(data + HEADER_WIRE_SIZE);
            if ((buttons & BUTTON_MASK) != buttons)
                return std::nullopt;
            return InputPacket{header, buttons};
        }
        case PacketType::SNAPSHOT: {
            if (size < SNAPSHOT_BASE_WIRE_SIZE)
                return std::nullopt;
            const auto count = readU16(data + HEADER_WIRE_SIZE + 4);
            if (count > MAX_SNAPSHOT_ENTITIES
                || size != SNAPSHOT_BASE_WIRE_SIZE + count * ENTITY_WIRE_SIZE)
                return std::nullopt;
            SnapshotPacket packet{header, readU32(data + HEADER_WIRE_SIZE), {}};
            packet.entities.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto* entry = data + SNAPSHOT_BASE_WIRE_SIZE + i * ENTITY_WIRE_SIZE;
                EntityState entity{readU32(entry), static_cast<EntityKind>(entry[4]),
                    std::bit_cast<float, uint32_t>(readU32(entry + 5)),
                    std::bit_cast<float, uint32_t>(readU32(entry + 9)),
                    readU16(entry + 13), entry[15]};
                if (!validEntity(entity))
                    return std::nullopt;
                packet.entities.push_back(entity);
            }
            return packet;
        }
        default:
            return std::nullopt;
    }
}
