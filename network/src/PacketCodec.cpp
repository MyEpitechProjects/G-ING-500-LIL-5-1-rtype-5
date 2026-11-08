#include "PacketCodec.hpp"
#include <cstring>

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
            if (size != HEADER_WIRE_SIZE + 4)
                return nullptr;
            std::unique_ptr<WelcomePacket> packet = std::make_unique<WelcomePacket>(header);
            packet->clientId = readU32(data + HEADER_WIRE_SIZE);
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
            const auto count = readU16(data + HEADER_WIRE_SIZE + 8);
            if (count > MAX_SNAPSHOT_ENTITIES
                || size != SNAPSHOT_BASE_WIRE_SIZE + count * ENTITY_WIRE_SIZE)
                return nullptr;
            std::unique_ptr<SnapshotPacket> packet = std::make_unique<SnapshotPacket>(header);
            packet->serverTick = readU32(data + HEADER_WIRE_SIZE);
            packet->score = readU32(data + HEADER_WIRE_SIZE + 4);
            packet->entities.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto* entry = data + SNAPSHOT_BASE_WIRE_SIZE + i * ENTITY_WIRE_SIZE;
                const std::uint32_t xBits = readU32(entry + 5);
                const std::uint32_t yBits = readU32(entry + 9);
                float x;
                float y;
                std::memcpy(&x, &xBits, sizeof(x));
                std::memcpy(&y, &yBits, sizeof(y));
                EntityState entity{readU32(entry), static_cast<EntityKind>(entry[4]),
                    x, y,
                    readU16(entry + 13), entry[15]};
                if (!validEntity(entity))
                    return nullptr;
                packet->entities.push_back(entity);
            }
            return packet;
        } case PacketType::CREATE_INSTANCE: {
            if (size != HEADER_WIRE_SIZE)
                return nullptr;
            return std::make_unique<CreateInstancePacket>(header);
        } case PacketType::INSTANCE: {
            if (size != INSTANCE_WIRE_SIZE)
                return nullptr;
            std::unique_ptr<InstancePacket> packet = std::make_unique<InstancePacket>(header);
            packet->instanceId = readU32(data + HEADER_WIRE_SIZE);
            return packet;
        } case PacketType::LIST_INSTANCES: {
            if (size != HEADER_WIRE_SIZE)
                return nullptr;
            return std::make_unique<ListInstancesPacket>(header);
        } case PacketType::INSTANCE_LIST: {
            if (size < INSTANCE_LIST_BASE_WIRE_SIZE)
                return nullptr;
            const auto count = readU16(data + HEADER_WIRE_SIZE);
            if (count > MAX_LISTED_INSTANCES
                || size != INSTANCE_LIST_BASE_WIRE_SIZE + count * INSTANCE_INFO_WIRE_SIZE)
                return nullptr;
            std::unique_ptr<InstanceListPacket> packet = std::make_unique<InstanceListPacket>(header);
            packet->instances.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                const auto* entry = data + INSTANCE_LIST_BASE_WIRE_SIZE + i * INSTANCE_INFO_WIRE_SIZE;
                packet->instances.push_back(InstanceInfo{readU32(entry), readU16(entry + 4)});
            }
            return packet;
        } case PacketType::EVENT: {
            if (size < EVENT_BASE_WIRE_SIZE)
                return nullptr;
            const EventType eventType = static_cast<EventType>(data[HEADER_WIRE_SIZE]);
            std::shared_ptr<const IEvent> eventData = deserializeEvent(eventType,
                data + EVENT_BASE_WIRE_SIZE, size - EVENT_BASE_WIRE_SIZE);
            if (!eventData)
                return nullptr;
            std::unique_ptr<EventPacket> packet = std::make_unique<EventPacket>(header);
            packet->eventType = eventType;
            packet->eventId = readU32(data + HEADER_WIRE_SIZE + 1);
            packet->eventData = std::move(eventData);
            return packet;
        }
        default:
            return nullptr;
    }
}

bool Net::PacketCodec::validEventData(EventType type, const IEvent &event) {
    switch (type) {
        case EventType::ACK:
            return dynamic_cast<const EmptyEvent*>(&event) != nullptr;
        case EventType::NEW_CLIENT:
        case EventType::DESTROY:
        case EventType::DISCONNECTED:
        case EventType::YOUR_ENTITY:
            return dynamic_cast<const EntityEvent*>(&event) != nullptr;
        case EventType::CHAT: {
            const auto* chat = dynamic_cast<const ChatEvent*>(&event);
            return chat != nullptr && chat->text.size() <= MAX_CHAT_LENGTH;
        }
        default:
            return false;
    }
}

std::shared_ptr<const Net::IEvent> Net::PacketCodec::deserializeEvent(EventType type, const std::uint8_t* data, std::size_t size) {
    switch (type) {
        case EventType::ACK:
            if (size != 0)
                return nullptr;
            return std::make_shared<EmptyEvent>();
        case EventType::NEW_CLIENT:
        case EventType::DESTROY:
        case EventType::DISCONNECTED:
        case EventType::YOUR_ENTITY:
            if (size != ENTITY_EVENT_WIRE_SIZE)
                return nullptr;
            return std::make_shared<EntityEvent>(readU32(data));
        case EventType::CHAT: {
            if (size < CHAT_EVENT_BASE_WIRE_SIZE)
                return nullptr;
            const std::uint16_t length = readU16(data + 4);
            if (length > MAX_CHAT_LENGTH || size != CHAT_EVENT_BASE_WIRE_SIZE + length)
                return nullptr;
            const auto* text = reinterpret_cast<const char*>(data + CHAT_EVENT_BASE_WIRE_SIZE);
            return std::make_shared<ChatEvent>(readU32(data), std::string(text, length));
        }
        default:
            return nullptr;
    }
}
