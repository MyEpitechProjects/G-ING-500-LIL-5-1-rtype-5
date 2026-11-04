/*
** EPITECH PROJECT, 2026
** ZipperIterator
** File description:
** ZipperIterator def
*/

#ifndef ZIPPERITERATOR
    #define ZIPPERITERATOR

    #include <iterator>
    #include <tuple>
    #include <utility>

namespace Containers {

template <class... Sources>
class Zipper;

template <class... Sources>
class ZipperIterator {
    private:
        static_assert(sizeof...(Sources) > 0);

        template <class Container>
        using IteratorType = decltype(std::declval<Container &>().begin());

        template <class Container>
        using ReferenceType = decltype(*std::declval<IteratorType<Container> &>());

        using IteratorTuple = std::tuple<IteratorType<Sources>...>;
        using ValueType = std::tuple<decltype(std::declval<ReferenceType<Sources>>().value())...>;

        friend class Containers::Zipper<Sources...>;

        IteratorTuple _current;
        std::size_t _max;
        std::size_t _index;
        static constexpr std::index_sequence_for<Sources...> _sequence{};

        ZipperIterator(IteratorTuple const &iterators, std::size_t max, std::size_t index = 0) : _current(iterators), _max(max), _index(index) {
            while (_index < _max && !allSet(_sequence))
                incrementAll(_sequence);
        }

        template <std::size_t... Is>
        void incrementAll(std::index_sequence<Is...>) {
            (++std::get<Is>(_current), ...);
            ++_index;
        }

        template <std::size_t... Is>
        bool allSet(std::index_sequence<Is...>) const {
            return (static_cast<bool>(*std::get<Is>(_current)) && ...);
        }

        template <std::size_t... Is>
        ValueType toValue(std::index_sequence<Is...>) const {
            return ValueType{std::get<Is>(_current)->value()...};
        }

    public:
        using value_type = ValueType;
        using reference = ValueType;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        ZipperIterator(ZipperIterator const &other) = default;

        ZipperIterator &operator++() {
            incrementAll(_sequence);
            while (_index < _max && !allSet(_sequence))
                incrementAll(_sequence);
            return *this;
        }

        ZipperIterator operator++(int) {
            ZipperIterator previous = *this;

            ++*this;
            return previous;
        }

        value_type operator*() const { return toValue(_sequence); }
        value_type operator->() const { return toValue(_sequence); }
        std::size_t getIndex() const { return _index; }

        friend bool operator==(ZipperIterator const &left, ZipperIterator const &right) {
            return left._index == right._index && left._current == right._current;
        }

        friend bool operator!=(ZipperIterator const &left, ZipperIterator const &right) {
            return !(left == right);
        }
};

}

#endif
