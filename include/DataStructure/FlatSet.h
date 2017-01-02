#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace ds {

// A set-like container backed up by an unsorted vector

template <typename T, typename Allocator = std::allocator<T>>
class FlatSet {
private:
    using VectorType = std::vector<T, Allocator>;
    VectorType vec;

public:
    // Member types
    using value_type = typename VectorType::value_type;
    using allocator_type = typename VectorType::allocator_type;
    using size_type = typename VectorType::size_type;
    using difference_type = typename VectorType::difference_type;
    using reference = typename VectorType::reference;
    using const_reference = typename VectorType::const_reference;
    using pointer = typename VectorType::pointer;
    using const_pointer = typename VectorType::const_pointer;
    using iterator = typename VectorType::iterator;
    using const_iterator = typename VectorType::const_iterator;
    using reverse_iterator = typename VectorType::reverse_iterator;
    using const_reverse_iterator = typename VectorType::const_reverse_iterator;

    FlatSet() = default;
    explicit FlatSet(const Allocator& a) : vec(a) {}
    template <typename Iterator>
    FlatSet(Iterator first, Iterator last, const Allocator& a = Allocator())
        : vec(first, last, a) {}
    FlatSet(std::initializer_list<T> init, const Allocator& a = Allocator())
        : vec(std::move(init), a) {}
    FlatSet(const std::vector<T, Allocator>& v) : vec(v) {}
    FlatSet(std::vector<T, Allocator>&& v) : vec(std::move(v)) {}
    FlatSet(const FlatSet&) = default;
    FlatSet(FlatSet&&) = default;
    FlatSet& operator=(const FlatSet&) = default;
    FlatSet& operator=(FlatSet&&) = default;

    template <typename Iterator>
    void assign(Iterator first, Iterator last) {
        vec.assign(first, last);
    }
    void assign(std::initializer_list<T> init) { vec.assign(std::move(init)); }

    // Capacity
    bool empty() const noexcept { return vec.empty(); }
    size_type size() const noexcept { return vec.size(); }
    size_type capacity() const noexcept { return vec.capacity(); }
    void reserve(size_type size) { vec.reserve(size); }
    void shrink_to_fit() { vec.shrink_to_fit(); }

    // Element access
    reference operator[](size_type pos) { return vec[pos]; }
    const_reference operator[](size_type pos) const { return vec[pos]; }
    reference at(size_type pos) { return vec.at(pos); }
    const_reference at(size_type pos) const { return vec.at(pos); }
    reference front() { return vec.front(); }
    const_reference front() const { return vec.front(); }
    reference back() { return vec.back(); }
    const_reference back() const { return vec.back(); }

    // Lookup
    iterator find(const T& elem) { return std::find(begin(), end(), elem); }
    const_iterator find(const T& elem) const {
        return std::find(begin(), end(), elem);
    }
    template <typename UnaryPredicate>
    iterator find_if(UnaryPredicate p) {
        return std::find_if(begin(), end(), std::move(p));
    }
    template <typename UnaryPredicate>
    const_iterator find_if(UnaryPredicate p) const {
        return std::find_if(begin(), end(), std::move(p));
    }
    size_type count(const T& elem) const { return find(elem) != end(); }

    // Compare operators
    template <typename U, typename A>
    friend bool operator==(const FlatSet<U, A>& lhs, const FlatSet<U, A>& rhs);
    template <typename U, typename A>
    friend bool operator!=(const FlatSet<U, A>& lhs, const FlatSet<U, A>& rhs);

    // Modifiers
    void clear() noexcept { vec.clear(); }
    void swap(FlatSet& rhs) noexcept(noexcept(vec.swap(rhs.vec))) {
        vec.swap(rhs.vec);
    }
    std::pair<iterator, bool> insert(const T& elem) {
        auto itr = find(elem);
        bool changed = false;
        if (itr == end()) {
            vec.push_back(elem);
            itr = std::prev(vec.end());
            changed = true;
        }
        return std::make_pair(itr, changed);
    }
    std::pair<iterator, bool> insert(T&& elem) {
        auto itr = find(elem);
        bool changed = false;
        if (itr == end()) {
            vec.push_back(std::move(elem));
            itr = std::prev(vec.end());
            changed = true;
        }
        return std::make_pair(itr, changed);
    }
    template <typename Iterator>
    void insert(Iterator first, Iterator last) {
        vec.reserve(vec.size() + std::distance(first, last));
        for (auto itr = first; itr != last; ++itr)
            insert(*itr);
    }
    void erase(iterator pos) {
        assert(pos != end());
        vec.erase(pos);
    }
    void erase(const_iterator pos) {
        assert(pos != end());
        vec.erase(pos);
    }
    void erase(iterator first, iterator last) { vec.erase(first, last); }
    void erase(const_iterator first, const_iterator last) {
        vec.erase(first, last);
    }
    size_type erase(const T& elem) {
        auto itr = find(elem);
        if (itr != end()) {
            erase(itr);
            return 1;
        } else
            return 0;
    }

    // Iterators
    iterator begin() { return vec.begin(); }
    iterator end() { return vec.end(); }
    const_iterator begin() const { return vec.begin(); }
    const_iterator end() const { return vec.end(); }
    const_iterator cbegin() const { return vec.cbegin(); }
    const_iterator cend() const { return vec.cend(); }
    reverse_iterator rbegin() { return vec.rbegin(); }
    reverse_iterator rend() { return vec.rend(); }
    const_reverse_iterator rbegin() const { return vec.rbegin(); }
    const_reverse_iterator rend() const { return vec.rend(); }
    const_reverse_iterator crbegin() const { return vec.crbegin(); }
    const_reverse_iterator crend() const { return vec.crend(); }
};

template <typename T, typename A>
bool operator==(const FlatSet<T, A>& lhs, const FlatSet<T, A>& rhs) {
    return lhs.vec == rhs.vec;
}

template <typename T, typename A>
bool operator!=(const FlatSet<T, A>& lhs, const FlatSet<T, A>& rhs) {
    return !(lhs == rhs);
}
}