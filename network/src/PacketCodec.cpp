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

Net::PacketCodec::Packet Net::PacketCodec::deserialize(const std::uint8_t* data, std::size_t size) {
    if (data == nullptr || size < HEADER_WIRE_SIZE || size > MAX_DATAGRAM_SIZE
        || data[1] != PROTOCOL_VERSION)
        return nullptr;
    const PacketHeader header{static_cast<PacketType>(data[0]), data[1],
        readU16(data + 2), readU32(data + 4)};
    switch (header.type) {
        case PacketType::HELLO: {
            if (size != HEADER_WIRE_SIZE)
                return nullptr;
            return std::make_unique<HelloPacket>(header);
        } case PacketType::WELCOME: {
            if (size != HEADER_WIRE_SIZE + 8)
                return nullptr;
            std::unique_ptr<WelcomePacket> packet = std::make_unique<WelcomePacket>(header);
            packet->playerEntityId = readU32(data + HEADER_WIRE_SIZE);
            packet->entityId = readU32(data + HEADER_WIRE_SIZE + sizeof(packet->playerEntityId));
            return packet;
        } case PacketType::INPUT: {
            if (size != HEADER_WIRE_SIZE + 2)
                return nullptr;
            std::unique_ptr<InputPacket> packet = std::make_unique<InputPacket>(header);
            const auto buttons = readU16(data + HEADER_WIRE_SIZE);
            if ((buttons & BUTTON_MASK) != buttons)
                return nullptr;
            packet->buttons = buttons;
            return packet;
        } case PacketType::SNAPSHOT: {
            if (size < SNAPSHOT_BASE_WIRE_SIZE)
                return nullptr;
            const auto count = readU16(data + HEADER_WIRE_SIZE + 4);
            if (count > MAX_SNAPSHOT_ENTITIES
                || size != SNAPSHOT_BASE_WIRE_SIZE + count * ENTITY_WIRE_SIZE)
                return nullptr;
            std::unique_ptr<SnapshotPacket> packet = std::make_unique<SnapshotPacket>(header);
            packet->serverTick = readU32(data + HEADER_WIRE_SIZE);
            packet->entities.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto* entry = data + SNAPSHOT_BASE_WIRE_SIZE + i * ENTITY_WIRE_SIZE;
                EntityState entity{readU32(entry), static_cast<EntityKind>(entry[4]),
                    std::bit_cast<float, uint32_t>(readU32(entry + 5)),
                    std::bit_cast<float, uint32_t>(readU32(entry + 9)),
                    readU16(entry + 13), entry[15]};
                if (!validEntity(entity))
                    return nullptr;
                packet->entities.push_back(entity);
            }
            return packet;
        } case PacketType::EVENT: {
            if (size != HEADER_WIRE_SIZE + 9)
                return nullptr;
            std::unique_ptr<GameEventPacket> packet = std::make_unique<GameEventPacket>(header);
            packet->eventType = static_cast<EventType>(data[HEADER_WIRE_SIZE]);
            if (packet->eventType != Net::EventType::ACK && packet->eventType != Net::EventType::DESTROY && packet->eventType != Net::EventType::DISCONNECTED && packet->eventType != Net::EventType::NEW_CLIENT)
                return nullptr;
            packet->entityId = readU32(data + HEADER_WIRE_SIZE + 1);
            packet->eventId = readU32(data + HEADER_WIRE_SIZE + 5);
            return packet;
        }
        default:
            return nullptr;
    }
}
