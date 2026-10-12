#ifndef SPARSE_ARRAY_HPP
    #define SPARSE_ARRAY_HPP

    #include <iostream>
    #include <optional>
    #include <vector>
    #include <memory>
    #include <stdexcept>
    #include <utility>

template <typename Component> // You can also mirror the definition of std :: vector ,that takes an additional allocator .
class SparseArray {
    public :

        using value_type = std::optional<Component>; // optional component type
        using reference_type = value_type &;
        using const_reference_type = value_type const &;
        using container_t = std::vector<value_type>; // optionally add your allocator template here .
        using size_type = typename container_t::size_type;
        using iterator = typename container_t::iterator;
        using const_iterator = typename container_t::const_iterator;

        SparseArray () = default;
        SparseArray (SparseArray const &) = default;
        SparseArray (SparseArray &&) noexcept = default;
        ~SparseArray() = default;
        SparseArray &operator=(SparseArray const &) = default;
        SparseArray &operator=(SparseArray &&) noexcept = default;
        reference_type operator [](size_t idx);
        const_reference_type operator [](size_t idx) const;
        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;
        iterator end();
        const_iterator end() const ;
        const_iterator cend() const ;
        size_type size() const ;
        reference_type insert_at ( size_type pos , Component const &component) ;
        reference_type insert_at ( size_type pos , Component &&component) ;
        //template < class ... Params >
        //reference_type emplace_at ( size_type pos , Params &&...) ; // optional
        void erase ( size_type pos ) ;
        size_type get_index ( value_type const &value) const ;

    private:
        container_t _data;
};


template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::operator[](std::size_t index) {
    return _data[index];
}

template <typename Component>
typename SparseArray<Component>::const_reference_type SparseArray<Component>::operator[](std::size_t index) const {
    return _data[index];
}

template <typename Component>
typename SparseArray<Component>::iterator SparseArray<Component>::begin() {
    return _data.begin();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::begin() const {
    return _data.begin();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::cbegin() const {
    return _data.cbegin();
}

template <typename Component>
typename SparseArray<Component>::iterator SparseArray<Component>::end() {
    return _data.end();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::end() const {
    return _data.end();
}

template <typename Component>
typename SparseArray<Component>::const_iterator SparseArray<Component>::cend() const {
    return _data.cend();
}

template <typename Component>
typename SparseArray<Component>::size_type SparseArray<Component>::size() const {
    return _data.size();
}

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::insert_at(size_type pos , Component const &component) {
    if (pos >= _data.size()) {
        _data.resize(pos + 1);
    }
    _data[pos] = component;
    return _data[pos];
}

template <typename Component>
typename SparseArray<Component>::reference_type SparseArray<Component>::insert_at(size_type pos , Component &&component) {
    if (pos >= _data.size()) {
        _data.resize(pos + 1);
    }
    _data[pos] = std::move(component);
    return _data[pos];
}

template <typename Component>
void SparseArray<Component>::erase(size_type pos) {
    if (pos < _data.size()) {
        _data[pos].reset();
    }
}

template <typename Component>
typename SparseArray<Component>::size_type SparseArray<Component>::get_index(value_type const &component) const {
    for (size_type i = 0; i < _data.size(); i++) {
        if (std::addressof(_data[i]) == std::addressof(component))
            return i;
    }
    throw std::out_of_range("not in sparse array");
}
#endif