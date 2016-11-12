#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>

namespace ds {

// This class implements an owning, unordered container that supports back
// insertion, deletion, and iteration.
// The primary guarantee it provides is that the storage of the elements in this
// collection are stable, meaning that once they get allocated, they will never
// get moved around. This means that pointers to those elements are stable and
// will never be invalidated unless the elements themselves are removed.

// Furthermore, the capacity of the container can grow dynamically.
// The stability of elements and dynamic growth of capacity come with the
// performance penalty on iteration, as elements are no longer stored in memory
// contiguously. Moreover, this container class supports neither random element
// access nor insertion at arbitrary location. Its iterators may be invalidated
// when after an insertion or deletion.
// As the name suggests, elements in UnorderedContainer are not ordered by any
// kind of indices. The only way to get access into the elements is to keep the
// reference returned by the create() function or dereferencing a valid
// iterator. In some sense, the "key" associated with each element is the
// element's own memory address.

// Under the hood the container is just a vector of unique_ptr. Although an
// std::list can also satisfy all the constraints mentioned above, the cost it
// pays (two additional pointers for each element) does not justify the
// additional benefit it provides (stable iterators) for some of the
// applications I had in mind.

// TODO: customize allocator and deleter

template <typename T>
class UnorderedCollectionIterator;
template <typename T>
class UnorderedCollectionConstIterator;

template <typename T>
class UnorderedCollection {
private:
    using ListElementType = std::unique_ptr<T>;
    using ListType = std::vector<ListElementType>;
    ListType allocList;

    friend class UnorderedCollectionIterator<T>;
    friend class UnorderedCollectionConstIterator<T>;

public:
    using value_type = T;
    using allocator_type = typename ListType::allocator_type;
    using deleter_type = typename ListElementType::deleter_type;
    using size_type = typename ListType::size_type;
    using difference_type = typename ListType::difference_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename ListElementType::pointer;
    using const_pointer = const pointer;
    using iterator = UnorderedCollectionIterator<T>;
    using const_iterator = UnorderedCollectionConstIterator<T>;

    UnorderedCollection() = default;
    explicit UnorderedCollection(size_type count) { allocList.reserve(count); }
    UnorderedCollection(std::initializer_list<T> init) {
        allocList.reserve(init.size());
        for (auto&& elem : init)
            allocList.emplace_back(std::make_unique<T>(std::move(elem)));
    }
    UnorderedCollection(const UnorderedCollection<T>& rhs) = delete;
    UnorderedCollection(UnorderedCollection<T>&& rhs) = default;
    UnorderedCollection<T>&
    operator=(const UnorderedCollection<T>& rhs) = delete;
    UnorderedCollection<T>& operator=(UnorderedCollection<T>&& rhs) = default;

    bool empty() const { return allocList.empty(); }
    size_type size() const { return allocList.size(); }

    void reserve(size_type sz) { allocList.reserve(sz); }
    void clear() { return allocList.clear(); }
    void swap(UnorderedCollection<T>& rhs) noexcept(
        noexcept(allocList.swap(rhs.allocList))) {
        allocList.swap(rhs.allocList);
    }

    template <typename... Args>
    T& create(Args&&... args) {
        allocList.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return *allocList.back();
    }

    // This function removes 'elem' from the collection and release its memory
    // immeidately.
    // It requires O(n) time for scaning the entire collection to find out the
    // index of 'elem'. The actual removal takes O(1) time, as we do not need to
    // maintain the order of the elements.
    void remove(const T& elem) {
        auto itr =
            std::find_if(allocList.begin(), allocList.end(),
                         [&elem](auto&& uptr) { return uptr.get() == &elem; });
        if (itr != allocList.end()) {
            std::swap(*itr, allocList.back());
            allocList.pop_back();
        }
    }

    // This function removes a batch of elements from the collection.
    // Under the hood, it create a new allocList and move all elements in the
    // old allocList to the new one except those that are in elemSet.
    // Here, elemSet is expected to be a set of T*. The runtime complexity of
    // this function is O(n*m), where O(m) is the complexity of elemSet.count().
    template <typename ElementSet>
    void remove_batch(const ElementSet& elemSet) {
        ListType newAllocList;
        newAllocList.reserve(allocList.size());
        for (auto&& elem : allocList) {
            if (!elemSet.count(elem.get()))
                newAllocList.emplace_back(std::move(elem));
        }
        allocList.swap(newAllocList);
    }

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
};

template <typename T>
class UnorderedCollectionIterator
    : public std::iterator<std::forward_iterator_tag, T> {
private:
    using RawIteratorType = typename UnorderedCollection<T>::ListType::iterator;
    RawIteratorType itr;

    using ParentType = std::iterator<std::forward_iterator_tag, T>;

public:
    using typename ParentType::iterator_category;
    using typename ParentType::reference;
    using typename ParentType::pointer;
    using typename ParentType::difference_type;

    UnorderedCollectionIterator() = default;
    UnorderedCollectionIterator(RawIteratorType i) : itr(i) {}

    UnorderedCollectionIterator<T> operator++(int) { return itr++; }
    UnorderedCollectionIterator<T>& operator++() {
        ++itr;
        return *this;
    }
    reference operator*() { return **itr; }
    pointer operator->() { return &(**itr); }
    UnorderedCollectionIterator<T> operator+(difference_type v) const {
        return itr + v;
    }
    bool operator==(const UnorderedCollectionIterator<T>& rhs) const {
        return itr == rhs.itr;
    }
    bool operator!=(const UnorderedCollectionIterator<T>& rhs) const {
        return !(*this == rhs);
    }
};

template <typename T>
class UnorderedCollectionConstIterator
    : public std::iterator<std::forward_iterator_tag, T> {
private:
    using RawIteratorType =
        typename UnorderedCollection<T>::ListType::const_iterator;
    RawIteratorType itr;

    using ParentType = std::iterator<std::forward_iterator_tag, T>;

public:
    using typename ParentType::iterator_category;
    using typename ParentType::reference;
    using typename ParentType::pointer;
    using typename ParentType::difference_type;

    UnorderedCollectionConstIterator() = default;
    UnorderedCollectionConstIterator(RawIteratorType i) : itr(i) {}

    UnorderedCollectionConstIterator<T> operator++(int) { return itr++; }
    UnorderedCollectionConstIterator<T>& operator++() {
        ++itr;
        return *this;
    }
    reference operator*() const { return **itr; }
    pointer operator->() const { return &(**itr); }
    UnorderedCollectionConstIterator<T> operator+(difference_type v) const {
        return itr + v;
    }
    bool operator==(const UnorderedCollectionConstIterator<T>& rhs) const {
        return itr == rhs.itr;
    }
    bool operator!=(const UnorderedCollectionConstIterator<T>& rhs) const {
        return !(*this == rhs);
    }
};

template <typename T>
typename UnorderedCollection<T>::iterator UnorderedCollection<T>::begin() {
    return iterator(allocList.begin());
}

template <typename T>
typename UnorderedCollection<T>::iterator UnorderedCollection<T>::end() {
    return iterator(allocList.end());
}

template <typename T>
typename UnorderedCollection<T>::const_iterator
UnorderedCollection<T>::begin() const {
    return const_iterator(allocList.cbegin());
}

template <typename T>
typename UnorderedCollection<T>::const_iterator
UnorderedCollection<T>::end() const {
    return const_iterator(allocList.cend());
}
}

namespace std {

template <typename T>
void swap(ds::UnorderedCollection<T>& lhs,
          ds::UnorderedCollection<T>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}
}