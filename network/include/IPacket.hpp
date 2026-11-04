#ifndef IPACKET_HPP
    #define IPACKET_HPP

    #include <cstdint>
    #include <vector>

class IPacket {
    public:
        virtual ~IPacket() = default;

        virtual std::vector<std::uint8_t> serialize() const = 0;
};

#endif
