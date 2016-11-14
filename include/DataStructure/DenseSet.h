#pragma once

#include "DataStructure/DenseMap.h"

namespace ds {

namespace detail {

struct DenseSetEmpty {};

template <typename KeyT>
class DenseSetPair : public DenseSetEmpty {
private:
    KeyT key;

public:
    KeyT& getFirst() { return key; }
    const KeyT& getFirst() const { return key; }
    DenseSetEmpty& getSecond() { return *this; }
    const DenseSetEmpty& getSecond() const { return *this; }
};
}

template <typename ValueT, typename ValueInfoT = DenseMapInfo<ValueT>>
class DenseSet {
private:
    using MapTy = DenseMap<ValueT, detail::DenseSetEmpty, ValueInfoT,
                           detail::DenseSetPair<ValueT>>;
    static_assert(sizeof(typename MapTy::value_type) == sizeof(ValueT),
                  "DenseMap buckets unexpectedly large!");
    MapTy theMap;

public:
    using key_type = ValueT;
    using value_type = ValueT;
    using size_type = unsigned;

    explicit DenseSet(unsigned numInitBuckets = 0) : theMap(numInitBuckets) {}
    DenseSet(std::initializer_list<ValueT> elems) : DenseSet(elems.size()) {
        insert(elems.begin(), elems.end());
    }
    bool empty() const { return theMap.empty(); }
    size_type size() const { return theMap.size(); }
    size_type getMemorySize() const { return theMap.getMemorySize(); }
    size_type count(const ValueT& v) const { return theMap.count(v); }
    bool erase(const ValueT& v) { return theMap.erase(v); }
    void swap(DenseSet& rhs) { theMap.swap(rhs.theMap); }
    void resize(size_t s) { theMap.resize(s); }
    void reserve(size_t s) { theMap.reserve(s); }
    void clear() { theMap.clear(); }
    class Iterator {
    private:
        typename MapTy::iterator itr;
        friend class DenseSet;

    public:
        using difference_type = typename MapTy::iterator::difference_type;
        using value_type = ValueT;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        Iterator(const typename MapTy::iterator& i) : itr(i) {}
        ValueT& operator*() { return itr->getFirst(); }
        ValueT* operator->() { return &itr->getFirst(); }
        Iterator& operator++() {
            ++itr;
            return *this;
        }
        bool operator==(const Iterator& rhs) const { return itr == rhs.itr; }
        bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }
    };

    class ConstIterator {
    private:
        typename MapTy::const_iterator itr;
        friend class DenseSet;

    public:
        using difference_type = typename MapTy::const_iterator::difference_type;
        using value_type = ValueT;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        ConstIterator(const typename MapTy::const_iterator& i) : itr(i) {}
        const ValueT& operator*() const { return itr->getFirst(); }
        const ValueT* operator->() const { return &itr->getFirst(); }
        ConstIterator& operator++() {
            ++itr;
            return *this;
        }
        bool operator==(const ConstIterator& rhs) const {
            return itr == rhs.itr;
        }
        bool operator!=(const ConstIterator& rhs) const {
            return !(*this == rhs);
        }
    };

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    iterator begin() { return Iterator(theMap.begin()); }
    iterator end() { return Iterator(theMap.end()); }
    const_iterator begin() const { return ConstIterator(theMap.begin()); }
    const_iterator end() const { return ConstIterator(theMap.end()); }
    iterator find(const ValueT& v) { return Iterator(theMap.find(v)); }
    const_iterator find(const ValueT& v) const {
        return ConstIterator(theMap.find(v));
    }
    template <typename LookupKeyT>
    iterator find_as(const LookupKeyT& key) {
        return Iterator(theMap.find_as(key));
    }
    template <typename LookupKeyT>
    const_iterator find_as(const LookupKeyT& key) const {
        return ConstIterator(theMap.find_as(key));
    }

    void erase(Iterator i) { return theMap.erase(i.itr); }
    void erase(ConstIterator i) { return theMap.erase(i.itr); }
    std::pair<iterator, bool> insert(const ValueT& v) {
        detail::DenseSetEmpty empty;
        return theMap.try_emplace(v, empty);
    }
    std::pair<iterator, bool> insert(ValueT&& v) {
        detail::DenseSetEmpty empty;
        return theMap.try_emplace(std::move(v), empty);
    }

    template <typename InputIt>
    void insert(InputIt itr, InputIt ite) {
        for (; itr != ite; ++itr)
            insert(*itr);
    }
};
}
