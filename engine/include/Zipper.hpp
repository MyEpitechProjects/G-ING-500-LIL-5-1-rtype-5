/*
** EPITECH PROJECT, 2026
** Zipper
** File description:
** Zipper def
*/

#ifndef ZIPPER
    #define ZIPPER

    #include "ZipperIterator.hpp"
    #include <algorithm>
    #include <iterator>
    #include <tuple>

namespace Containers {

template <class... Sources>
class Zipper {
    private:
        using IteratorTuple = typename Containers::ZipperIterator<Sources...>::IteratorTuple;

        IteratorTuple _begin;
        IteratorTuple _end;
        std::size_t _size;

        static std::size_t computeSize(Sources &...containers) {
            return std::min({containers.size()...});
        }

        static IteratorTuple computeEnd(Sources &...containers) {
            const std::size_t size = computeSize(containers...);

            return IteratorTuple(std::next(containers.begin(), static_cast<std::ptrdiff_t>(size))...);
        }

    public:
        using iterator = Containers::ZipperIterator<Sources...>;

        Zipper(Sources &...containers) : _begin(containers.begin()...), _end(computeEnd(containers...)), _size(computeSize(containers...)) {}

        iterator begin() { return iterator(_begin, _size); }
        iterator end() { return iterator(_end, _size, _size); }
};

}

#endif
