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
    PacketCodec::writeU32(out, clientId);
    return out;
}

std::vector<std::uint8_t> Net::CreateInstancePacket::serialize() const {
    std::vector<std::uint8_t> out;
    out.reserve(HEADER_WIRE_SIZE);
    PacketCodec::writeHeader(out, header, PacketType::CREATE_INSTANCE);
    return out;
}

std::vector<std::uint8_t> Net::InstancePacket::serialize() const {
    std::vector<std::uint8_t> out;
    out.reserve(INSTANCE_WIRE_SIZE);
    PacketCodec::writeHeader(out, header, PacketType::INSTANCE);
    PacketCodec::writeU32(out, instanceId);
    return out;
}

std::vector<std::uint8_t> Net::EventPacket::serialize() const {
    const std::optional<std::size_t> payloadSize = PacketCodec::eventPayloadSize(eventType);
    if (!payloadSize || !eventData)
        throw std::invalid_argument("Invalid event type");
    const std::size_t expectedSize = EVENT_BASE_WIRE_SIZE + payloadSize.value();
    std::vector<std::uint8_t> out;
    out.reserve(expectedSize);
    PacketCodec::writeHeader(out, header, PacketType::EVENT);
    out.push_back(static_cast<std::uint8_t>(eventType));
    PacketCodec::writeU32(out, eventId);
    eventData->serialize(out);
    if (out.size() != expectedSize)
        throw std::invalid_argument("Event data does not match event type");
    return out;
}

void Net::EmptyEvent::serialize([[maybe_unused]] std::vector<std::uint8_t> &out) const {
}

Net::EntityEvent::EntityEvent(std::uint32_t id) : entityId(id) {
}

void Net::EntityEvent::serialize(std::vector<std::uint8_t> &out) const {
    PacketCodec::writeU32(out, entityId);
}
