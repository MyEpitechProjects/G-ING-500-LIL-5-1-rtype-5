/*
** EPITECH PROJECT, 2026
** SparseArray
** File description:
** SparseArray def
*/

#ifndef SPARSEARRAY
    #define SPARSEARRAY

    #include <exception>
    #include <memory>
    #include <optional>
    #include <string>
    #include <utility>
    #include <vector>

template <typename Component>
class SparseArray {
    private:
        std::vector<std::optional<Component>> _data;

    public:
        using value_type = std::optional<Component>;
        using reference_type = value_type &;
        using const_reference_type = value_type const &;
        using container_t = std::vector<value_type>;
        using size_type = typename container_t::size_type;
        using iterator = typename container_t::iterator;
        using const_iterator = typename container_t::const_iterator;

        SparseArray() = default;
        SparseArray(SparseArray const &) = default;
        SparseArray(SparseArray &&) noexcept = default;
        ~SparseArray() = default;
        SparseArray &operator=(SparseArray const &) = default;
        SparseArray &operator=(SparseArray &&) noexcept = default;

        reference_type operator[](size_type index) { return _data[index]; }
        const_reference_type operator[](size_type index) const { return _data[index]; }

        iterator begin() { return _data.begin(); }
        const_iterator begin() const { return _data.begin(); }
        const_iterator cbegin() const { return _data.cbegin(); }
        iterator end() { return _data.end(); }
        const_iterator end() const { return _data.end(); }
        const_iterator cend() const { return _data.cend(); }
        size_type size() const { return _data.size(); }

        reference_type insertAt(size_type position, Component const &component);
        reference_type insertAt(size_type position, Component &&component);
        void erase(size_type position);
        size_type getIndex(value_type const &value) const;

        class Exception : public std::exception {
            private:
                std::string _message;
            public:
                Exception(const std::string &message) : _message(message) {}
                const char *what() const noexcept override { return _message.c_str(); }
        };
};

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::insertAt(size_type position, Component const &component) {
    if (position >= _data.size())
        _data.resize(position + 1);
    _data[position] = component;
    return _data[position];
}

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::insertAt(size_type position, Component &&component) {
    if (position >= _data.size())
        _data.resize(position + 1);
    _data[position] = std::move(component);
    return _data[position];
}

template <typename Component>
void SparseArray<Component>::erase(size_type position) {
    if (position < _data.size())
        _data[position].reset();
}

template <typename Component>
typename SparseArray<Component>::size_type SparseArray<Component>::getIndex(value_type const &value) const {
    for (size_type index = 0; index < _data.size(); index++)
        if (std::addressof(_data[index]) == std::addressof(value))
            return index;
    throw Exception("SparseArray: component is not in the array");
}

#endif
