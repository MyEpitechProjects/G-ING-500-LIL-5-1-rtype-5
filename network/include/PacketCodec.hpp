#ifndef PACKET_CODEC_HPP
    #define PACKET_CODEC_HPP

    #include <vector>
    #include <optional>
    #include <variant>
    #include <cstdint>
    #include <bit>
    #include <cmath>
    #include <limits>
    #include <stdexcept>

    #include "Paquet.hpp"

namespace Net {

    constexpr std::uint16_t BUTTON_MASK = Net::UP | Net::DOWN | Net::LEFT | Net::RIGHT;

    class PacketCodec{
        private:
        
            static void writeU16(std::vector<std::uint8_t>& out, std::uint16_t value);
            static void writeU32(std::vector<std::uint8_t>& out, std::uint32_t value);

            static std::uint16_t readU16(const std::uint8_t* data);
            static std::uint32_t readU32(const std::uint8_t* data);
            static void writeHeader(std::vector<std::uint8_t>& out,
                const PacketHeader& header, PacketType expected);

            static bool validEntity(const Net::EntityState& entity);

        public:
            PacketCodec() = default;
            ~PacketCodec() = default;

            using Packet = std::variant<InputPacket, SnapshotPacket,
                HelloPacket, WelcomePacket>;

            // Big-endian wire format. Invalid outgoing fields throw std::invalid_argument.
            static std::vector<std::uint8_t> serialize(const InputPacket& packet);
            static std::vector<std::uint8_t> serialize(const SnapshotPacket& packet);
            static std::vector<std::uint8_t> serialize(const HelloPacket& packet);
            static std::vector<std::uint8_t> serialize(const WelcomePacket& packet);

            // Rejects unknown types/versions, invalid fields and non-exact sizes.
            static std::optional<Packet> deserialize(const std::uint8_t* data,
                std::size_t size);
            static std::optional<Packet> deserialize(const std::vector<std::uint8_t>& data)
            {
                return deserialize(data.data(), data.size());
            }
    };
}

#endif
