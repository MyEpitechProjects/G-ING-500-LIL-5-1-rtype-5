#ifndef PAQUET_HPP
    #define PAQUET_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "IPacket.hpp"

namespace Net {

    constexpr std::uint8_t PROTOCOL_VERSION = 1;
    constexpr std::size_t MAX_DATAGRAM_SIZE = 1200;
    constexpr std::size_t HEADER_WIRE_SIZE = 8;
    constexpr std::size_t ENTITY_WIRE_SIZE = 16;
    constexpr std::size_t SNAPSHOT_BASE_WIRE_SIZE = HEADER_WIRE_SIZE + 6;
    constexpr std::size_t MAX_SNAPSHOT_ENTITIES = (MAX_DATAGRAM_SIZE - SNAPSHOT_BASE_WIRE_SIZE) / ENTITY_WIRE_SIZE;

    enum PacketType : std::uint8_t {
        INPUT = 1,
        SNAPSHOT = 2,
        HELLO = 3,
        WELCOME = 4,
    };

    enum InputButton : std::uint16_t {
        UP = 1 << 0,
        DOWN = 1 << 1,
        LEFT = 1 << 2,
        RIGHT = 1 << 3,
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
        std::uint32_t playerEntityId = 0;

        std::vector<std::uint8_t> serialize() const override;
    };
}

#endif
