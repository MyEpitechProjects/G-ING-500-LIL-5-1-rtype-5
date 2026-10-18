#ifndef ZIPPER_HPP
    #define ZIPPER_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>

namespace containers {
    template <class... Containers>
    class zipper;
}

template <class... Containers>
class zipper_iterator {
    static_assert(sizeof...(Containers) > 0);

    template <class Container>
    using iterator_t = decltype(std::declval<Container &>().begin());

    template <class Container>
    using it_reference_t = decltype(*std::declval<iterator_t<Container> &>());

    public:
        using value_type = std::tuple<decltype(std::declval<it_reference_t<Containers>>().value())...>;
        using reference = value_type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;
        using iterator_tuple = std::tuple<iterator_t<Containers>...>;

    private:
        friend class containers::zipper<Containers...>;
        zipper_iterator(iterator_tuple const &it_tuple, std::size_t max, std::size_t index = 0) : _current(it_tuple), _max(max), _idx(index) {
            while (_idx < _max && !all_set(_seq))
                incr_all(_seq);
        }

    public:
        zipper_iterator(zipper_iterator const &z) = default;

        zipper_iterator &operator++() {
            incr_all(_seq);
            while (_idx < _max && !all_set(_seq))
                incr_all(_seq);
            return *this;
        }

        zipper_iterator operator++(int) {
            zipper_iterator previous = *this;
            ++*this;
            return previous;
        }

        value_type operator*() const { return to_value(_seq); }
        value_type operator->() const { return to_value(_seq); }
        std::size_t index() const { return _idx; }

        friend bool operator==(zipper_iterator const &lhs, zipper_iterator const &rhs) {
            return lhs._idx == rhs._idx && lhs._current == rhs._current;
        }

        friend bool operator!=(zipper_iterator const &lhs, zipper_iterator const &rhs) {
            return !(lhs == rhs);
        }

    private:
        template <std::size_t... Is>
        void incr_all(std::index_sequence<Is...>) {
            (++std::get<Is>(_current), ...);
            ++_idx;
        }

        template <std::size_t... Is>
        bool all_set(std::index_sequence<Is...>) const {
            return (static_cast<bool>(*std::get<Is>(_current)) && ...);
        }

        template <std::size_t... Is>
        value_type to_value(std::index_sequence<Is...>) const {
            return value_type{std::get<Is>(_current)->value()...};
        }

    private:
        iterator_tuple _current;
        std::size_t _max;
        std::size_t _idx;
        static constexpr std::index_sequence_for<Containers...> _seq{};
};

namespace containers {
    template <class... Containers>
    class zipper {
        public:
            using iterator = zipper_iterator<Containers...>;
            using iterator_tuple = typename iterator::iterator_tuple;

            zipper(Containers &...containers) : _begin(containers.begin()...), _end(_compute_end(containers...)), _size(_compute_size(containers...)) {}

            iterator begin() { return iterator(_begin, _size); }
            iterator end() { return iterator(_end, _size, _size); }

        private:
            static std::size_t _compute_size(Containers &...containers) {
                return std::min({containers.size()...});
            }

            static iterator_tuple _compute_end(Containers &...containers) {
                const std::size_t size = _compute_size(containers...);
                return iterator_tuple(std::next(containers.begin(),
                    static_cast<std::ptrdiff_t>(size))...);
            }

            iterator_tuple _begin;
            iterator_tuple _end;
            std::size_t _size;
    };

}

#endif
