#pragma once

#include "DataStructure/Detail.h"
#include "DataStructure/StringView.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iterator>

namespace ds {

namespace detail {

unsigned getMinBucketToReserveForEntries(unsigned numEntries) {
    if (numEntries == 0)
        return 0;
    return nextPowerOfTwo(numEntries * 4 / 3 + 1);
}
}

template <typename ValueT>
class StringMapEntry {
private:
    unsigned strLen;

    StringMapEntry(const StringMapEntry&) = delete;

public:
    ValueT second;

    using first_type = StringView;
    using second_type = ValueT;

    explicit StringMapEntry(unsigned l) : strLen(l), second() {}
    template <typename... InitT>
    StringMapEntry(unsigned l, InitT&&... vs)
        : strLen(l), second(std::forward<InitT>(vs)...) {}

    StringView getKey() const { return StringView(getKeyData(), strLen); }
    unsigned getKeyLength() const { return strLen; }
    const ValueT& getValue() const { return second; }
    ValueT& getValue() { return second; }
    void setValue(const ValueT& v) { second = v; }
    const char* getKeyData() const {
        return reinterpret_cast<const char*>(this + 1);
    }
    StringView first() const {
        return StringView(getKeyData(), getKeyLength());
    }

    template <typename... InitT>
    static StringMapEntry* create(StringView key, InitT&&... vs) {
        unsigned keyLen = key.size();
        unsigned allocSize =
            static_cast<unsigned>(sizeof(StringMapEntry)) + keyLen + 1;
        StringMapEntry* newItem =
            static_cast<StringMapEntry*>(std::malloc(allocSize));
        new (newItem) StringMapEntry(keyLen, std::forward<InitT>(vs)...);

        char* strBuffer = const_cast<char*>(newItem->getKeyData());
        if (keyLen > 0)
            std::memcpy(strBuffer, key.data(), keyLen);
        strBuffer[keyLen] = 0;
        return newItem;
    }

    static StringMapEntry& getStringMapEntryFromKeyData(const char* keyData) {
        char* ptr = const_cast<char*>(keyData) - sizeof(StringMapEntry<ValueT>);
        return *reinterpret_cast<StringMapEntry*>(ptr);
    }

    void destroy() {
        this->~StringMapEntry();
        std::free(static_cast<void*>(this));
    }
};

template <typename ValueT>
class StringMapIterator;
template <typename ValueT>
class StringMapConstIterator;

template <typename ValueT>
class StringMap {
private:
    using MapEntry = StringMapEntry<ValueT>;

    MapEntry** theTable;
    unsigned numBuckets;
    unsigned numItems;
    unsigned numTombstones;
    unsigned itemSize;

    static unsigned hashString(StringView str, unsigned result = 0) {
        for (size_t i = 0, e = str.size(); i != e; ++i)
            result = result * 33 + (unsigned char)str[i];
        return result;
    }

    void init(unsigned initSize) {
        assert((initSize & (initSize - 1)) == 0 &&
               "initSize must be a power of 2");
        numBuckets = initSize ? initSize : 16;
        numItems = 0;
        numTombstones = 0;
        theTable = static_cast<MapEntry**>(
            std::calloc(numBuckets + 1, sizeof(MapEntry**) + sizeof(unsigned)));

        theTable[numBuckets] = reinterpret_cast<MapEntry*>(2);
    }

    unsigned lookupBucketFor(StringView name) {
        unsigned htSize = numBuckets;
        if (htSize == 0) {
            init(16);
            htSize = numBuckets;
        }

        unsigned fullHashValue = hashString(name);
        unsigned bucketNo = fullHashValue & (htSize - 1);
        unsigned* hashTable = (unsigned*)(theTable + numBuckets + 1);

        unsigned probeAmt = 1;
        int firstTombstone = -1;
        while (1) {
            MapEntry* bucketItem = theTable[bucketNo];
            if (!bucketItem) {
                if (firstTombstone != -1) {
                    hashTable[firstTombstone] = fullHashValue;
                    return firstTombstone;
                }
                hashTable[bucketNo] = fullHashValue;
                return bucketNo;
            }

            if (bucketItem == getTombstoneVal()) {
                if (firstTombstone == -1)
                    firstTombstone = bucketNo;
            } else if (hashTable[bucketNo] == fullHashValue) {
                char* itemStr = (char*)bucketItem + itemSize;
                if (name == StringView(itemStr, bucketItem->getKeyLength()))
                    return bucketNo;
            }

            bucketNo = (bucketNo + probeAmt) & (htSize - 1);
            ++probeAmt;
        }
    }

    int findKey(StringView key) const {
        unsigned htSize = numBuckets;
        if (htSize == 0)
            return -1;
        unsigned fullHashValue = hashString(key);
        unsigned bucketNo = fullHashValue & (htSize - 1);
        unsigned* hashTable = (unsigned*)(theTable + numBuckets + 1);

        unsigned probeAmt = 1;
        while (1) {
            MapEntry* bucketItem = theTable[bucketNo];
            if (!bucketItem)
                return -1;

            if (bucketItem != getTombstoneVal() &&
                hashTable[bucketNo] == fullHashValue) {
                char* itemStr = (char*)bucketItem + itemSize;
                if (key == StringView(itemStr, bucketItem->getKeyLength()))
                    return bucketNo;
            }

            bucketNo = (bucketNo + probeAmt) & (htSize - 1);
            ++probeAmt;
        }
    }

    MapEntry* removeKey(StringView key) {
        int bucket = findKey(key);
        if (bucket == -1)
            return nullptr;

        MapEntry* result = theTable[bucket];
        theTable[bucket] = getTombstoneVal();
        --numItems;
        ++numTombstones;
        assert(numItems + numTombstones <= numBuckets);

        return result;
    }

    void removeKey(MapEntry* v) {
        const char* vstr = (char*)v + itemSize;
        auto v2 = removeKey(StringView(vstr, v->getKeyLength()));
        (void)v2;
        assert(v == v2 && "Didn't find key?");
    }

    unsigned rehashTable(unsigned bucketNo = 0) {
        unsigned newSize;
        unsigned* hashTable = (unsigned*)(theTable + numBuckets + 1);

        if (numItems * 4 > numBuckets * 3)
            newSize = numBuckets * 2;
        else if ((numBuckets - (numItems + numTombstones)) <= numBuckets / 8)
            newSize = numBuckets;
        else
            return bucketNo;

        unsigned newBucketNo = bucketNo;
        MapEntry** newTableArray = static_cast<MapEntry**>(
            std::calloc(newSize + 1, sizeof(MapEntry*) + sizeof(unsigned)));
        unsigned* newHashArray = (unsigned*)(newTableArray + newSize + 1);
        newTableArray[newSize] = reinterpret_cast<MapEntry*>(2);

        for (unsigned i = 0, e = numBuckets; i != e; ++i) {
            MapEntry* bucket = theTable[i];
            if (bucket && bucket != getTombstoneVal()) {
                unsigned fullhash = hashTable[i];
                unsigned newBucket = fullhash & (newSize - 1);
                if (!newTableArray[newBucket]) {
                    newTableArray[fullhash & (newSize - 1)] = bucket;
                    newHashArray[fullhash & (newSize - 1)] = fullhash;
                    if (i == bucketNo)
                        newBucketNo = newBucket;
                    continue;
                }

                unsigned probeSize = 1;
                do {
                    newBucket = (newBucket + probeSize) & (newSize - 1);
                    ++probeSize;
                } while (newTableArray[newBucket]);

                newTableArray[newBucket] = bucket;
                newHashArray[newBucket] = fullhash;
                if (i == bucketNo)
                    newBucketNo = newBucket;
            }
        }

        free(theTable);
        theTable = newTableArray;
        numBuckets = newSize;
        numTombstones = 0;
        return newBucketNo;
    }

    bool insert(MapEntry* entry) {
        unsigned bucketNo = lookupBucketFor(entry->getKey());
        MapEntry*& bucket = theTable[bucketNo];
        if (bucket && bucket != getTombstoneVal())
            return false;

        if (bucket == getTombstoneVal())
            --numTombstones;
        bucket = entry;
        ++numItems;
        assert(numItems + numTombstones <= numBuckets);

        rehashTable();
        return true;
    }

public:
    using key_type = const char*;
    using mapped_type = ValueT;
    using value_type = MapEntry;
    using size_type = unsigned;
    using iterator = StringMapIterator<ValueT>;
    using const_iterator = StringMapConstIterator<ValueT>;

    StringMap(unsigned initSize, unsigned i) : itemSize(i) {
        if (initSize) {
            init(detail::getMinBucketToReserveForEntries(initSize));
            return;
        }

        theTable = nullptr;
        numBuckets = 0;
        numItems = 0;
        numTombstones = 0;
    }
    StringMap()
        : theTable(nullptr), numBuckets(0), numItems(0), numTombstones(0),
          itemSize(static_cast<unsigned>(sizeof(MapEntry))) {}
    explicit StringMap(unsigned i)
        : StringMap(i, static_cast<unsigned>(sizeof(MapEntry))) {}
    StringMap(StringMap&& rhs) noexcept
        : theTable(rhs.theTable), numBuckets(rhs.numBuckets),
          numItems(rhs.numItems), numTombstones(rhs.numTombstones),
          itemSize(rhs.itemSize) {
        rhs.theTable = nullptr;
        rhs.numBuckets = 0;
        rhs.numItems = 0;
        rhs.numTombstones = 0;
    }
    ~StringMap() {
        if (!empty()) {
            for (unsigned i = 0, e = numBuckets; i != e; ++i) {
                MapEntry* bucket = theTable[i];
                if (bucket && bucket != getTombstoneVal())
                    static_cast<MapEntry*>(bucket)->destroy();
                bucket = nullptr;
            }
        }
        free(theTable);
    }
    StringMap& operator=(StringMap rhs) {
        swap(rhs);
        return *this;
    }

    unsigned getNumBuckets() const { return numBuckets; }
    bool empty() const { return numItems == 0; }
    size_type size() const { return numItems; }
    void swap(StringMap& rhs) noexcept {
        std::swap(theTable, rhs.theTable);
        std::swap(numBuckets, rhs.numBuckets);
        std::swap(numItems, rhs.numItems);
        std::swap(numTombstones, rhs.numTombstones);
    }

    iterator begin() { return iterator(theTable, numBuckets == 0); }
    iterator end() { return iterator(theTable + numBuckets, true); }
    const_iterator begin() const {
        return const_iterator(theTable, numBuckets == 0);
    }
    const_iterator end() const {
        return const_iterator(theTable + numBuckets, true);
    }

    static MapEntry* getTombstoneVal() {
        return reinterpret_cast<MapEntry*>(-1);
    }

    iterator find(StringView key) {
        int bucket = findKey(key);
        if (bucket == -1)
            return end();
        return iterator(theTable + bucket, true);
    }

    const_iterator find(StringView key) const {
        int bucket = findKey(key);
        if (bucket == -1)
            return end();
        return iterator(theTable + bucket, true);
    }

    ValueT lookup(StringView key) const {
        auto it = find(key);
        if (it != end())
            return it->second;
        else
            return ValueT();
    }

    ValueT& operator[](StringView key) {
        return try_emplace(key).first->second;
    }
    size_type count(StringView key) const { return find(key) == end() ? 0 : 1; }
    std::pair<iterator, bool> insert(std::pair<StringView, ValueT> kv) {
        return try_emplace(kv.first, std::move(kv.second));
    }

    template <typename... Args>
    std::pair<iterator, bool> try_emplace(StringView key, Args&&... args) {
        unsigned bucketNo = lookupBucketFor(key);
        MapEntry*& bucket = theTable[bucketNo];
        if (bucket && bucket != getTombstoneVal())
            return std::make_pair(iterator(theTable + bucketNo, false), false);

        if (bucket == getTombstoneVal())
            --numTombstones;
        bucket = MapEntry::create(key, std::forward<Args>(args)...);
        ++numItems;
        assert(numItems + numTombstones <= numBuckets);

        bucketNo = rehashTable(bucketNo);
        return std::make_pair(iterator(theTable + bucketNo, false), true);
    }

    void clear() {
        if (empty())
            return;

        for (unsigned i = 0, e = numBuckets; i != e; ++i) {
            MapEntry*& bucket = theTable[i];
            if (bucket && bucket != getTombstoneVal())
                static_cast<MapEntry*>(bucket)->destroy();
            bucket = nullptr;
        }

        numItems = 0;
        numTombstones = 0;
    }

    void erase(iterator i) {
        MapEntry& v = *i;
        removeKey(&v);
        v.destroy();
    }

    bool erase(StringView key) {
        auto i = find(key);
        if (i == end())
            return false;
        erase(i);
        return true;
    }
};

template <typename ValueT>
class StringMapConstIterator {
protected:
    using MapEntry = StringMapEntry<ValueT>;

    MapEntry** ptr;

    void advancePastEmptyBuckets() {
        while (*ptr == nullptr || *ptr == StringMap<ValueT>::getTombstoneVal())
            ++ptr;
    }

public:
    using difference_type = std::ptrdiff_t;
    using pointer = MapEntry*;
    using reference = MapEntry&;
    using value_type = MapEntry;
    using iterator_category = std::forward_iterator_tag;

    StringMapConstIterator() : ptr(nullptr) {}
    explicit StringMapConstIterator(MapEntry** bucket, bool noAdvance = false)
        : ptr(bucket) {
        if (!noAdvance)
            advancePastEmptyBuckets();
    }

    const value_type& operator*() const {
        return *static_cast<MapEntry*>(*ptr);
    }
    const value_type* operator->() const {
        return static_cast<MapEntry*>(*ptr);
    }

    bool operator==(const StringMapConstIterator& rhs) const {
        return ptr == rhs.ptr;
    }
    bool operator!=(const StringMapConstIterator& rhs) const {
        return ptr != rhs.ptr;
    }

    StringMapConstIterator& operator++() {
        ++ptr;
        advancePastEmptyBuckets();
        return *this;
    }
    StringMapConstIterator operator++(int) {
        StringMapConstIterator tmp = *this;
        ++*this;
        return tmp;
    }
};

template <typename ValueT>
class StringMapIterator : public StringMapConstIterator<ValueT> {
private:
    using MapEntry = StringMapEntry<ValueT>;

public:
    using difference_type = std::ptrdiff_t;
    using pointer = MapEntry*;
    using reference = MapEntry&;
    using value_type = MapEntry;
    using iterator_category = std::forward_iterator_tag;

    StringMapIterator() {}
    explicit StringMapIterator(MapEntry** bucket, bool noAdvance = false)
        : StringMapConstIterator<ValueT>(bucket, noAdvance) {}

    value_type& operator*() const {
        return *static_cast<MapEntry*>(*this->ptr);
    }
    value_type* operator->() const {
        return static_cast<MapEntry*>(*this->ptr);
    }
};
}

namespace std {
template <typename T>
struct iterator_traits<ds::StringMapConstIterator<T>> {
    using difference_type =
        typename ds::StringMapConstIterator<T>::difference_type;
    using pointer = typename ds::StringMapConstIterator<T>::pointer;
    using reference = typename ds::StringMapConstIterator<T>::reference;
    using value_type = typename ds::StringMapConstIterator<T>::value_type;
    using iterator_category =
        typename ds::StringMapConstIterator<T>::iterator_category;
};

template <typename T>
struct iterator_traits<ds::StringMapIterator<T>> {
    using difference_type = typename ds::StringMapIterator<T>::difference_type;
    using pointer = typename ds::StringMapIterator<T>::pointer;
    using reference = typename ds::StringMapIterator<T>::reference;
    using value_type = typename ds::StringMapIterator<T>::value_type;
    using iterator_category =
        typename ds::StringMapIterator<T>::iterator_category;
};
}
