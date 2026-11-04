#ifndef PAQUET_HPP
    #define PAQUET_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include "IEvent.hpp"
#include "IPacket.hpp"

namespace Net {

    constexpr std::uint8_t PROTOCOL_VERSION = 1;
    constexpr std::size_t MAX_DATAGRAM_SIZE = 1200;
    constexpr std::size_t HEADER_WIRE_SIZE = 8;
    constexpr std::size_t ENTITY_WIRE_SIZE = 16;
    constexpr std::size_t SNAPSHOT_BASE_WIRE_SIZE = HEADER_WIRE_SIZE + 6;
    constexpr std::size_t MAX_SNAPSHOT_ENTITIES = (MAX_DATAGRAM_SIZE - SNAPSHOT_BASE_WIRE_SIZE) / ENTITY_WIRE_SIZE;
    constexpr std::size_t INSTANCE_WIRE_SIZE = HEADER_WIRE_SIZE + 4;
    constexpr std::uint32_t NO_INSTANCE = 0;
    constexpr std::size_t EVENT_BASE_WIRE_SIZE = HEADER_WIRE_SIZE + 5;
    constexpr std::size_t ENTITY_EVENT_WIRE_SIZE = 4;
    constexpr std::size_t INSTANCE_LIST_BASE_WIRE_SIZE = HEADER_WIRE_SIZE + 2;
    constexpr std::size_t INSTANCE_INFO_WIRE_SIZE = 6;
    constexpr std::size_t MAX_LISTED_INSTANCES = (MAX_DATAGRAM_SIZE - INSTANCE_LIST_BASE_WIRE_SIZE) / INSTANCE_INFO_WIRE_SIZE;
    constexpr std::size_t SERVER_TICK_RATE = 20;
    constexpr std::size_t CHAT_EVENT_BASE_WIRE_SIZE = 6;
    constexpr std::size_t MAX_CHAT_LENGTH = 256;

    enum PacketType : std::uint8_t {
        INPUT = 1,
        SNAPSHOT = 2,
        HELLO = 3,
        WELCOME = 4,
        CREATE_INSTANCE = 5,
        INSTANCE = 6,
        EVENT = 7,
        LIST_INSTANCES = 8,
        INSTANCE_LIST = 9,
    };

    enum EventType : std::uint8_t {
        NEW_CLIENT = 1,
        DESTROY = 2,
        ACK = 3,
        DISCONNECTED = 4,
        CHAT = 5,
        YOUR_ENTITY = 6,
    };

    enum InputButton : std::uint16_t {
        UP = 1 << 0,
        DOWN = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3,
        FIRE = 1 << 4,
    };

    enum EntityKind : std::uint8_t {
        PLAYER = 1,
        ENEMY = 2,
        PROJECTILE = 3,
    };

    enum EntityFlag : std::uint8_t {
        ALIVE = 1 << 0,
        INVULNERABLE = 1 << 1,
    };

    /*
    ** PACKET HEADER
    */

    struct PacketHeader {
        PacketType type;
        std::uint8_t version = PROTOCOL_VERSION;
        std::uint16_t sequence = 0;
        std::uint32_t sessionId = 0;
    };

    struct InputPacket : IPacket {
        InputPacket() = default;
        InputPacket(PacketHeader header_packet) : header(header_packet) {}
        ~InputPacket() override = default;

        PacketHeader header{PacketType::INPUT};
        std::uint16_t buttons = 0;

        std::vector<std::uint8_t> serialize() const override;
    };
    constexpr std::size_t INSTANCE_LIST_BASE_WIRE_SIZE = HEADER_WIRE_SIZE + 2;

    /*
    ** SNAPSHOT PACKET
    */

    struct EntityState {
        std::uint32_t id = 0;
        EntityKind kind = EntityKind::PLAYER;
        float x = 0.0f;
        float y = 0.0f;
        std::uint16_t health = 0;
        std::uint8_t flags = 0;
    };

    struct SnapshotPacket : IPacket {
        SnapshotPacket() = default;
        SnapshotPacket(PacketHeader header_packet) : header(header_packet) {}
        ~SnapshotPacket() override = default;

        PacketHeader header{PacketType::SNAPSHOT};
        std::uint32_t serverTick = 0;
        std::vector<EntityState> entities;

        std::vector<std::uint8_t> serialize() const override;
    };

    /*
    ** HELLO PACKET
    */

    struct HelloPacket : IPacket {
        HelloPacket() = default;
        HelloPacket(PacketHeader header_packet) : header(header_packet) {}
        ~HelloPacket() override = default;

        PacketHeader header{PacketType::HELLO};

        std::vector<std::uint8_t> serialize() const override;
    };

    /*
    ** WELCOME PACKET
    */

    struct WelcomePacket : IPacket {
        WelcomePacket() = default;
        WelcomePacket(PacketHeader header_packet) : header(header_packet) {}
        ~WelcomePacket() override = default;

        PacketHeader header{PacketType::WELCOME};
        std::uint32_t clientId = 0;

        std::vector<std::uint8_t> serialize() const override;
    };

    /*
    ** INSTANCE PACKETS
    */

    struct CreateInstancePacket : IPacket {
        CreateInstancePacket() = default;
        CreateInstancePacket(PacketHeader header_packet) : header(header_packet) {}
        ~CreateInstancePacket() override = default;

        PacketHeader header{PacketType::CREATE_INSTANCE};

        std::vector<std::uint8_t> serialize() const override;
    };

    struct InstancePacket : IPacket {
        InstancePacket() = default;
        InstancePacket(PacketHeader header_packet) : header(header_packet) {}
        ~InstancePacket() override = default;

        PacketHeader header{PacketType::INSTANCE};
        std::uint32_t instanceId = NO_INSTANCE;

        std::vector<std::uint8_t> serialize() const override;
    };


    /*
    ** INSTANCE LIST PACKETS
    */

    struct ListInstancesPacket : IPacket {
        ListInstancesPacket() = default;
        ListInstancesPacket(PacketHeader header_packet) : header(header_packet) {}
        ~ListInstancesPacket() override = default;

        PacketHeader header{PacketType::LIST_INSTANCES};

        std::vector<std::uint8_t> serialize() const override;
    };

    struct InstanceInfo {
        std::uint32_t id = NO_INSTANCE;
        std::uint16_t players = 0;
    };

    struct InstanceListPacket : IPacket {
        InstanceListPacket() = default;
        InstanceListPacket(PacketHeader header_packet) : header(header_packet) {}
        ~InstanceListPacket() override = default;

        PacketHeader header{PacketType::INSTANCE_LIST};
        std::vector<InstanceInfo> instances;

        std::vector<std::uint8_t> serialize() const override;
    };

    /*
    ** EVENT DATA
    */

    struct EmptyEvent : IEvent {
        EmptyEvent() = default;
        ~EmptyEvent() override = default;

        void serialize(std::vector<std::uint8_t> &out) const override;
    };

    struct EntityEvent : IEvent {
        EntityEvent() = default;
        explicit EntityEvent(std::uint32_t id);
        ~EntityEvent() override = default;

        std::uint32_t entityId = 0;

        void serialize(std::vector<std::uint8_t> &out) const override;
    };

    struct ChatEvent : IEvent {
        ChatEvent() = default;
        ChatEvent(std::uint32_t sender, std::string message);
        ~ChatEvent() override = default;

        std::uint32_t senderId = 0;
        std::string text;

        void serialize(std::vector<std::uint8_t> &out) const override;
    };

    /*
    ** EVENT PACKET
    */

    struct EventPacket : IPacket {
        EventPacket() = default;
        EventPacket(PacketHeader header_packet) : header(header_packet) {}
        ~EventPacket() override = default;

        PacketHeader header{PacketType::EVENT};
        EventType eventType = EventType::ACK;
        std::uint32_t eventId = 0;
        std::shared_ptr<const IEvent> eventData = std::make_shared<EmptyEvent>();

        std::vector<std::uint8_t> serialize() const override;
    };
}

#endif
