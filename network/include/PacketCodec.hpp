#ifndef PACKET_CODEC_HPP
    #define PACKET_CODEC_HPP

    #include <vector>
    #include <optional>
    #include <memory>
    #include <cstdint>
    #include <cmath>
    #include <limits>
    #include <stdexcept>

    #include "Paquet.hpp"

namespace Net {

    constexpr std::uint16_t BUTTON_MASK = Net::InputButton::UP | Net::InputButton::DOWN | Net::InputButton::LEFT | Net::InputButton::RIGHT | Net::InputButton::FIRE;

    class PacketCodec{
        public:
            PacketCodec() = default;
            ~PacketCodec() = default;

            using Packet = std::shared_ptr<IPacket>;

            static void writeU16(std::vector<std::uint8_t>& out, std::uint16_t value);
            static void writeU32(std::vector<std::uint8_t>& out, std::uint32_t value);

            static std::uint16_t readU16(const std::uint8_t* data);
            static std::uint32_t readU32(const std::uint8_t* data);
            static void writeHeader(std::vector<std::uint8_t>& out,
                const Net::PacketHeader& header, Net::PacketType expected);

            static bool validEntity(const Net::EntityState& entity);

            static Packet deserialize(const std::uint8_t* data, std::size_t size);
            static Packet deserialize(const std::vector<std::uint8_t>& data)
            {
                return deserialize(data.data(), data.size());
            }
    };
}

#endif
