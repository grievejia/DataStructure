#pragma once

#include "DataStructure/DenseMapInfo.h"
#include "DataStructure/Detail.h"

#include <cstring>

namespace ds {

namespace detail {

template <typename KeyT, typename ValueT>
struct DenseMapPair : public std::pair<KeyT, ValueT> {
    KeyT& getFirst() { return std::pair<KeyT, ValueT>::first; }
    const KeyT& getFirst() const { return std::pair<KeyT, ValueT>::first; }
    ValueT& getSecond() { return std::pair<KeyT, ValueT>::second; }
    const ValueT& getSecond() const { return std::pair<KeyT, ValueT>::second; }
};
}

template <
    typename KeyT, typename ValueT, typename KeyInfoT = DenseMapInfo<KeyT>,
    typename BucketT = detail::DenseMapPair<KeyT, ValueT>, bool IsConst = false>
class DenseMapIterator;

template <typename KeyT, typename ValueT,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
class DenseMap {
private:
    BucketT* buckets;
    unsigned numEntries;
    unsigned numTombstones;
    unsigned numBuckets;

    BucketT* getBuckets() { return buckets; }
    const BucketT* getBuckets() const { return buckets; }
    BucketT* getBucketsEnd() { return getBuckets() + numBuckets; }
    const BucketT* getBucketsEnd() const { return getBuckets() + numBuckets; }
    void destroyAll() {
        if (numBuckets == 0)
            return;

        auto emptyKey = KeyInfoT::getEmptyKey(),
             tombKey = KeyInfoT::getTombstoneKey();
        for (auto p = getBuckets(), e = getBucketsEnd(); p != e; ++p) {
            if (!KeyInfoT::isEqual(p->getFirst(), emptyKey) &&
                !KeyInfoT::isEqual(p->getFirst(), tombKey))
                p->getSecond().~ValueT();
            p->getFirst().~KeyT();
        }
    }

    void initEmpty() {
        assert((numBuckets & (numBuckets - 1)) == 0 &&
               "# initial buckets must be a power of 2");

        numEntries = 0;
        numTombstones = 0;

        auto emptyKey = KeyInfoT::getEmptyKey();
        for (auto b = getBuckets(), e = getBucketsEnd(); b != e; ++b)
            ::new (&b->getFirst()) KeyT(emptyKey);
    }

    void moveFromOldBuckets(BucketT* oldBegin, BucketT* oldEnd) {
        initEmpty();

        auto emptyKey = KeyInfoT::getEmptyKey(),
             tombKey = KeyInfoT::getTombstoneKey();
        for (auto b = oldBegin, e = oldEnd; b != e; ++b) {
            if (!KeyInfoT::isEqual(b->getFirst(), emptyKey) &&
                !KeyInfoT::isEqual(b->getFirst(), tombKey)) {
                BucketT* dstBucket;
                bool found = lookupBucketFor(b->getFirst(), dstBucket);
                (void)found; // silence warning
                assert(!found && "Key already in new map?");
                dstBucket->getFirst() = std::move(b->getFirst());
                ::new (&dstBucket->getSecond())
                    ValueT(std::move(b->getSecond()));
                ++numEntries;

                b->getSecond().~ValueT();
            }
            b->getFirst().~KeyT();
        }
    }

    void copyFromImpl(const DenseMap<KeyT, ValueT, KeyInfoT, BucketT>& other) {
        assert(&other != this);
        assert(numBuckets == other.numBuckets);

        numEntries = other.numEntries;
        numTombstones = other.numTombstones;

        if (std::is_pod<KeyT>::value && std::is_pod<KeyT>::value)
            std::memcpy(buckets, other.buckets, numBuckets * sizeof(BucketT));
        else {
            auto emptyKey = KeyInfoT::getEmptyKey(),
                 tombKey = KeyInfoT::getTombstoneKey();
            for (unsigned i = 0; i < numBuckets; ++i) {
                auto& dst = buckets[i].getFirst();
                ::new (&dst) KeyT(other.buckets[i].getFirst());
                if (dst != emptyKey && dst != tombKey)
                    ::new (&buckets[i].getSecond())
                        ValueT(other.buckets[i].getSecond());
            }
        }
    }

    BucketT* insertIntoBucket(const KeyT& key, const ValueT& value,
                              BucketT* theBucket) {
        theBucket = insertIntoBucketImpl(key, theBucket);
        theBucket->getFirst() = key;
        ::new (&theBucket->getSecond()) ValueT(value);
        return theBucket;
    }
    BucketT* insertIntoBucket(const KeyT& key, ValueT&& value,
                              BucketT* theBucket) {
        theBucket = insertIntoBucketImpl(key, theBucket);
        theBucket->getFirst() = key;
        ::new (&theBucket->getSecond()) ValueT(std::move(value));
        return theBucket;
    }
    BucketT* insertIntoBucket(KeyT&& key, ValueT&& value, BucketT* theBucket) {
        theBucket = insertIntoBucketImpl(key, theBucket);
        theBucket->getFirst() = std::move(key);
        ::new (&theBucket->getSecond()) ValueT(std::move(value));
        return theBucket;
    }
    BucketT* insertIntoBucketImpl(const KeyT& key, BucketT* theBucket) {
        auto newNumEntries = numEntries + 1;
        if (newNumEntries * 4 >= numBuckets * 3) {
            grow(numBuckets * 2);
            lookupBucketFor(key, theBucket);
        } else if ((numBuckets - (newNumEntries + numTombstones)) <=
                   numBuckets / 8) {
            grow(numBuckets);
            lookupBucketFor(key, theBucket);
        }
        assert(theBucket);

        ++numEntries;

        if (!KeyInfoT::isEqual(theBucket->getFirst(), KeyInfoT::getEmptyKey()))
            --numTombstones;
        return theBucket;
    }

    bool lookupBucketFor(const KeyT& k, const BucketT*& foundBucket) const {
        if (numBuckets == 0) {
            foundBucket = nullptr;
            return false;
        }

        const BucketT* foundTomb = nullptr;
        auto emptyKey = KeyInfoT::getEmptyKey(),
             tombKey = KeyInfoT::getTombstoneKey();
        assert(!KeyInfoT::isEqual(k, emptyKey) &&
               !KeyInfoT::isEqual(k, tombKey) &&
               "empty/tombstone value shouldn't be inserted into map!");

        unsigned bucketNo = KeyInfoT::getHashValue(k) & (numBuckets - 1);
        unsigned probeAmt = 1;
        while (true) {
            const BucketT* thisBucket = buckets + bucketNo;
            if (KeyInfoT::isEqual(k, thisBucket->getFirst())) {
                foundBucket = thisBucket;
                return true;
            }

            if (KeyInfoT::isEqual(thisBucket->getFirst(), emptyKey)) {
                foundBucket = foundTomb ? foundTomb : thisBucket;
                return false;
            }

            if (KeyInfoT::isEqual(thisBucket->getFirst(), tombKey) &&
                !foundTomb)
                foundTomb = thisBucket;

            bucketNo += probeAmt++;
            bucketNo &= (numBuckets - 1);
        }
    }

    bool lookupBucketFor(const KeyT& key, BucketT*& foundBucket) {
        const BucketT* constFoundBucket;
        bool result = const_cast<const DenseMap*>(this)->lookupBucketFor(
            key, constFoundBucket);
        foundBucket = const_cast<BucketT*>(constFoundBucket);
        return result;
    }

    bool allocateBuckets(unsigned num) {
        numBuckets = num;
        if (numBuckets == 0) {
            buckets = nullptr;
            return false;
        }

        buckets =
            static_cast<BucketT*>(operator new(sizeof(BucketT) * numBuckets));
        return true;
    }

    void copyFrom(const DenseMap& rhs) {
        destroyAll();
        operator delete(buckets);
        if (allocateBuckets(rhs.numBuckets))
            copyFromImpl(rhs);
        else {
            numEntries = 0;
            numTombstones = 0;
        }
    }

    void init(unsigned numInitBuckets) {
        if (allocateBuckets(numInitBuckets))
            initEmpty();
        else {
            numEntries = 0;
            numTombstones = 0;
        }
    }

    void grow(unsigned atLeast) {
        auto oldNumBuckets = numBuckets;
        BucketT* oldBuckets = buckets;

        allocateBuckets(std::max<unsigned>(
            64, static_cast<unsigned>(detail::nextPowerOfTwo(atLeast - 1))));
        assert(buckets);
        if (!oldBuckets) {
            initEmpty();
            return;
        }

        moveFromOldBuckets(oldBuckets, oldBuckets + oldNumBuckets);
        operator delete(oldBuckets);
    }

    void shrink_and_clear() {
        auto oldNumEntries = numEntries;
        destroyAll();

        unsigned newNumBuckets = 0;
        if (oldNumEntries)
            newNumBuckets =
                std::max(64, 1 << (detail::log2_32_ceil(oldNumEntries) + 1));
        if (newNumBuckets == numBuckets) {
            initEmpty();
            return;
        }

        operator delete(buckets);
        init(newNumBuckets);
    }

public:
    using size_type = unsigned;
    using key_type = KeyT;
    using value_type = BucketT;
    using mapped_type = ValueT;

    using iterator = DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT>;
    using const_iterator =
        DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, true>;

    explicit DenseMap(unsigned numInitBuckets = 0) { init(numInitBuckets); }
    DenseMap(const DenseMap& rhs) {
        init(0);
        copyFrom(rhs);
    }
    DenseMap& operator=(const DenseMap& rhs) {
        if (&rhs != this)
            copyFrom(rhs);
        return *this;
    }
    DenseMap(DenseMap&& rhs) noexcept {
        init(0);
        swap(rhs);
    }
    DenseMap& operator=(DenseMap&& rhs) noexcept {
        destroyAll();
        operator delete(buckets);
        init(0);
        swap(rhs);
        return *this;
    }
    template <typename Iterator>
    DenseMap(const Iterator& i, const Iterator& e) {
        init(detail::nextPowerOfTwo(std::distance(i, e)));
        insert(i, e);
    }
    DenseMap(std::initializer_list<value_type> init) {
        init(detail::nextPowerOfTwo(init.size()));
        insert(init.begin(), init.end());
    }
    ~DenseMap() {
        destroyAll();
        operator delete(buckets);
    }

    void swap(DenseMap& rhs) {
        std::swap(buckets, rhs.buckets);
        std::swap(numEntries, rhs.numEntries);
        std::swap(numTombstones, rhs.numTombstones);
        std::swap(numBuckets, rhs.numBuckets);
    }

    void resize(size_type s) {
        if (s > numBuckets)
            grow(s);
    }

    void clear() {
        if (numEntries == 0 && numTombstones == 0)
            return;

        if (size() * 4 < numBuckets && numBuckets > 64) {
            shrink_and_clear();
            return;
        }

        auto emptyKey = KeyInfoT::getEmptyKey(),
             tombKey = KeyInfoT::getTombstoneKey();
        for (auto p = getBuckets(), e = getBucketsEnd(); p != e; ++p) {
            if (!KeyInfoT::isEqual(p->getFirst(), emptyKey)) {
                if (!KeyInfoT::isEqual(p->getFirst(), tombKey)) {
                    p->getSecond().~ValueT();
                    --numEntries;
                }
                p->getFirst() = emptyKey;
            }
        }
        assert(numEntries == 0 && "Node count imbalance!");
        numTombstones = 0;
    }

    size_type count(const KeyT& k) const {
        const BucketT* theBucket;
        return lookupBucketFor(k, theBucket) ? 1 : 0;
    }

    iterator find(const KeyT& k) {
        BucketT* theBucket;
        if (lookupBucketFor(k, theBucket))
            return iterator(theBucket, getBucketsEnd(), true);
        return end();
    }
    const_iterator find(const KeyT& k) const {
        const BucketT* theBucket;
        if (lookupBucketFor(k, theBucket))
            return const_iterator(theBucket, getBucketsEnd(), true);
        return end();
    }
    ValueT lookup(const KeyT& k) const {
        const BucketT* theBucket;
        if (lookupBucketFor(k, theBucket))
            return theBucket->getSecond();
        return ValueT();
    }
    ValueT at(const KeyT& k) const {
        const BucketT* theBucket;
        if (lookupBucketFor(k, theBucket))
            return theBucket->getSecond();
        throw std::out_of_range("DenseMap lookup failed");
    }

    std::pair<iterator, bool> insert(const std::pair<KeyT, ValueT>& kv) {
        BucketT* theBucket;
        if (lookupBucketFor(kv.first, theBucket))
            return std::make_pair(iterator(theBucket, getBucketsEnd(), true),
                                  false);

        theBucket = insertIntoBucket(kv.first, kv.second, theBucket);
        return std::make_pair(iterator(theBucket, getBucketsEnd(), true), true);
    }
    std::pair<iterator, bool> insert(std::pair<KeyT, ValueT>&& kv) {
        BucketT* theBucket;
        if (lookupBucketFor(kv.first, theBucket))
            return std::make_pair(iterator(theBucket, getBucketsEnd(), true),
                                  false);

        theBucket = insertIntoBucket(std::move(kv.first), std::move(kv.second),
                                     theBucket);
        return std::make_pair(iterator(theBucket, getBucketsEnd(), true), true);
    }

    template <typename Iterator>
    void insert(Iterator i, Iterator e) {
        for (; i != e; ++i)
            insert(*i);
    }

    value_type& findAndConstruct(const KeyT& k) {
        BucketT* theBucket;
        if (lookupBucketFor(k, theBucket))
            return *theBucket;
        return *insertIntoBucket(k, ValueT(), theBucket);
    }
    value_type& findAndConstruct(KeyT&& k) {
        BucketT* theBucket;
        if (lookupBucketFor(k, theBucket))
            return *theBucket;
        return *insertIntoBucket(std::move(k), ValueT(), theBucket);
    }
    ValueT& operator[](const KeyT& k) { return findAndConstruct(k).second; }
    ValueT& operator[](KeyT&& k) {
        return findAndConstruct(std::move(k)).second;
    }

    bool erase(const KeyT& k) {
        BucketT* theBucket;
        if (!lookupBucketFor(k, theBucket))
            return false;

        theBucket->getSecond().~ValueT();
        theBucket->getFirst() = KeyInfoT::getTombstoneKey();
        --numEntries;
        ++numTombstones;
        return true;
    }
    void erase(iterator i) {
        BucketT* theBucket = &*i;
        theBucket->getSecond().~ValueT();
        theBucket->getFirst() = KeyInfoT::getTombstoneKey();
        --numEntries;
        ++numTombstones;
    }

    bool empty() const { return numEntries == 0; }
    size_t size() const { return numEntries; }
    iterator begin() {
        return empty() ? end() : iterator(getBuckets(), getBucketsEnd());
    }
    iterator end() { return iterator(getBucketsEnd(), getBucketsEnd(), true); }
    const_iterator begin() const {
        return empty() ? end() : const_iterator(getBuckets(), getBucketsEnd());
    }
    const_iterator end() const {
        return const_iterator(getBucketsEnd(), getBucketsEnd(), true);
    }
};

template <typename KeyT, typename ValueT, typename KeyInfoT, typename BucketT,
          bool IsConst>
class DenseMapIterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type =
        typename std::conditional_t<IsConst, const BucketT, BucketT>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

private:
    using ConstIterator =
        DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, true>;
    friend class DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, true>;
    friend class DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, false>;

    pointer ptr, end;

    void advancePastEmptyBuckets() {
        auto emptyKey = KeyInfoT::getEmptyKey(),
             tombKey = KeyInfoT::getTombstoneKey();
        while (ptr != end && (KeyInfoT::isEqual(ptr->getFirst(), emptyKey) ||
                              KeyInfoT::isEqual(ptr->getFirst(), tombKey)))
            ++ptr;
    }

public:
    DenseMapIterator() : ptr(nullptr), end(nullptr) {}
    DenseMapIterator(pointer pos, pointer e, bool noAdvance = false)
        : ptr(pos), end(e) {
        if (!noAdvance)
            advancePastEmptyBuckets();
    }
    template <bool IsConstSrc,
              typename = typename std::enable_if_t<!IsConstSrc && IsConst>>
    DenseMapIterator(
        const DenseMapIterator<KeyT, ValueT, KeyInfoT, BucketT, IsConstSrc>& i)
        : ptr(i.ptr), end(i.end) {}

    reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }
    bool operator==(const ConstIterator& rhs) const { return ptr == rhs.ptr; }
    bool operator!=(const ConstIterator& rhs) const { return !(*this == rhs); }
    DenseMapIterator& operator++() {
        ++ptr;
        advancePastEmptyBuckets();
        return *this;
    }
    DenseMapIterator operator++(int) {
        DenseMapIterator ret = *this;
        ++*this;
        return ret;
    }
};
}
