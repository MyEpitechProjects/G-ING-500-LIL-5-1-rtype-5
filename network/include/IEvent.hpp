#ifndef IEVENT_HPP
    #define IEVENT_HPP

    #include <cstdint>
    #include <vector>

namespace Net {
    class IEvent {
        public:
            virtual ~IEvent() = default;

            virtual void serialize(std::vector<std::uint8_t> &out) const = 0;
    };
}

#endif
