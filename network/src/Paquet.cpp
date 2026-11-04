#include "Paquet.hpp"
#include "PacketCodec.hpp"
#include <cstring>
#include <utility>

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
    if (!eventData || !PacketCodec::validEventData(eventType, *eventData))
        throw std::invalid_argument("Event data does not match event type");
    std::vector<std::uint8_t> out;
    out.reserve(EVENT_BASE_WIRE_SIZE);
    PacketCodec::writeHeader(out, header, PacketType::EVENT);
    out.push_back(static_cast<std::uint8_t>(eventType));
    PacketCodec::writeU32(out, eventId);
    eventData->serialize(out);
    if (out.size() > MAX_DATAGRAM_SIZE)
        throw std::invalid_argument("Event packet too large");
    return out;
}

void Net::EmptyEvent::serialize([[maybe_unused]] std::vector<std::uint8_t> &out) const {
}

Net::EntityEvent::EntityEvent(std::uint32_t id) : entityId(id) {
}

void Net::EntityEvent::serialize(std::vector<std::uint8_t> &out) const {
    PacketCodec::writeU32(out, entityId);
}

Net::ChatEvent::ChatEvent(std::uint32_t sender, std::string message) : senderId(sender), text(std::move(message)) {
}

void Net::ChatEvent::serialize(std::vector<std::uint8_t> &out) const {
    if (text.size() > MAX_CHAT_LENGTH)
        throw std::invalid_argument("Chat message too long");
    PacketCodec::writeU32(out, senderId);
    PacketCodec::writeU16(out, static_cast<std::uint16_t>(text.size()));
    out.insert(out.end(), text.begin(), text.end());
}
