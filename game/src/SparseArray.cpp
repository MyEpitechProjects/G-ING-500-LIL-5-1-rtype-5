#include "SparseArray.hpp"

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