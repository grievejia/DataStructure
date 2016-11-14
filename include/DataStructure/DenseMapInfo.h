#pragma once

#include "DataStructure/ArrayRef.h"
#include "DataStructure/StringView.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ds {

template <typename T>
struct DenseMapInfo {
    // static inline T getEmptyKey();
    // static inline T getTombstoneKey();
    // static unsigned getHashValue(const T&);
    // static bool isEqual(const T& lhs, const T& rhs);
};

template <typename T>
struct DenseMapInfo<T*> {
    // Assume pointers are 4-type aligned
    static constexpr unsigned NumLowBitsAvailable = 2;

    static inline T* getEmptyKey() {
        uintptr_t val = static_cast<uintptr_t>(-1);
        val <<= NumLowBitsAvailable;
        return reinterpret_cast<T*>(val);
    }

    static inline T* getTombstoneKey() {
        uintptr_t val = static_cast<uintptr_t>(-2);
        val <<= NumLowBitsAvailable;
        return reinterpret_cast<T*>(val);
    }

    static unsigned getHashValue(const T* p) {
        return (unsigned((uintptr_t)p) >> 4) ^ (unsigned((uintptr_t)p) >> 9);
    }

    static bool isEqual(const T* lhs, const T* rhs) { return lhs == rhs; }
};

template <>
struct DenseMapInfo<char> {
    static inline char getEmptyKey() { return ~0; }
    static inline char getTombstoneKey() { return ~0 - 1; }
    static unsigned getHashValue(const char& c) { return c * 37U; }
    static bool isEqual(const char lhs, const char rhs) { return lhs == rhs; }
};

template <>
struct DenseMapInfo<unsigned> {
    static inline unsigned getEmptyKey() { return ~0U; }
    static inline unsigned getTombstoneKey() { return ~0U - 1; }
    static unsigned getHashValue(const unsigned& v) { return v * 37U; }
    static bool isEqual(const unsigned lhs, const unsigned rhs) {
        return lhs == rhs;
    }
};

template <>
struct DenseMapInfo<unsigned long> {
    static inline unsigned long getEmptyKey() { return ~0UL; }
    static inline unsigned long getTombstoneKey() { return ~0UL - 1L; }
    static unsigned getHashValue(const unsigned long& v) {
        return (unsigned)(v * 37UL);
    }
    static bool isEqual(const unsigned long lhs, const unsigned long rhs) {
        return lhs == rhs;
    }
};

template <>
struct DenseMapInfo<unsigned long long> {
    static inline unsigned long long getEmptyKey() { return ~0ULL; }
    static inline unsigned long long getTombstoneKey() { return ~0ULL - 1ULL; }
    static unsigned getHashValue(const unsigned long long& v) {
        return (unsigned)(v * 37ULL);
    }
    static bool isEqual(const unsigned long long lhs,
                        const unsigned long long rhs) {
        return lhs == rhs;
    }
};

template <>
struct DenseMapInfo<int> {
    static inline int getEmptyKey() { return 0x7fffffff; }
    static inline int getTombstoneKey() { return -0x7fffffff - 1; }
    static unsigned getHashValue(const int& v) { return (unsigned)(v * 37U); }
    static bool isEqual(const int lhs, const int rhs) { return lhs == rhs; }
};

template <>
struct DenseMapInfo<long> {
    static inline long getEmptyKey() {
        return (1UL << (sizeof(long) * 8 - 1)) - 1UL;
    }
    static inline long getTombstoneKey() { return getEmptyKey() - 1L; }
    static unsigned getHashValue(const long& v) { return (unsigned)(v * 37UL); }
    static bool isEqual(const long lhs, const long rhs) { return lhs == rhs; }
};

template <>
struct DenseMapInfo<long long> {
    static inline long long getEmptyKey() { return 0x7fffffffffffffffLL; }
    static inline long long getTombstoneKey() {
        return -0x7fffffffffffffffLL - 1;
    }
    static unsigned getHashValue(const long long& v) {
        return (unsigned)(v * 37ULL);
    }
    static bool isEqual(const long long lhs, const long long rhs) {
        return lhs == rhs;
    }
};

inline unsigned denseMapHashCombine(unsigned lhs, unsigned rhs) {
    uint64_t key =
        (static_cast<uint64_t>(lhs) << 32) | static_cast<uint64_t>(rhs);
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    return (unsigned)key;
}

template <typename T, typename U>
unsigned denseMapHashPair(const T& first, const U& second) {
    return denseMapHashCombine(DenseMapInfo<T>::getHashValue(first),
                               DenseMapInfo<U>::getHashValue(second));
};

template <typename T, typename U, typename V>
unsigned denseMapHashTriple(const T& first, const U& second, const V& third) {
    return denseMapHashCombine(
        denseMapHashCombine(DenseMapInfo<T>::getHashValue(first),
                            DenseMapInfo<U>::getHashValue(second)),
        DenseMapInfo<V>::getHashValue(third));
};

template <typename T, typename U, typename V, typename W>
unsigned denseMapHashQuadraple(const T& first, const U& second, const V& third,
                               const W& fourth) {
    return denseMapHashCombine(
        denseMapHashCombine(
            denseMapHashCombine(DenseMapInfo<T>::getHashValue(first),
                                DenseMapInfo<U>::getHashValue(second)),
            DenseMapInfo<V>::getHashValue(third)),
        DenseMapInfo<W>::getHashValue(fourth));
};

template <typename T, typename U>
struct DenseMapInfo<std::pair<T, U>> {
    using Pair = std::pair<T, U>;
    using FirstInfo = DenseMapInfo<T>;
    using SecondInfo = DenseMapInfo<U>;

    static inline Pair getEmptyKey() {
        return std::make_pair(FirstInfo::getEmptyKey(),
                              SecondInfo::getEmptyKey());
    }

    static inline Pair getTombstoneKey() {
        return std::make_pair(FirstInfo::getTombstoneKey(),
                              SecondInfo::getTombstoneKey());
    }

    static unsigned getHashValue(const Pair& p) {
        return denseMapHashPair(p.first, p.second);
    }

    static bool isEqual(const Pair& lhs, const Pair& rhs) {
        return FirstInfo::isEqual(lhs.first, rhs.first) &&
               SecondInfo::isEqual(lhs.second, rhs.second);
    }
};

template <typename T, typename U, typename V>
struct DenseMapInfo<std::tuple<T, U, V>> {
    using Triple = std::tuple<T, U, V>;
    using FirstInfo = DenseMapInfo<T>;
    using SecondInfo = DenseMapInfo<U>;
    using ThirdInfo = DenseMapInfo<V>;

    static inline Triple getEmptyKey() {
        return std::make_tuple(FirstInfo::getEmptyKey(),
                               SecondInfo::getEmptyKey(),
                               ThirdInfo::getEmptyKey());
    }

    static inline Triple getTombstoneKey() {
        return std::make_tuple(FirstInfo::getTombstoneKey(),
                               SecondInfo::getTombstoneKey(),
                               ThirdInfo::getTombstoneKey());
    }

    static unsigned getHashValue(const Triple& p) {
        return denseMapHashTriple(std::get<0>(p), std::get<1>(p),
                                  std::get<2>(p));
    }

    static bool isEqual(const Triple& lhs, const Triple& rhs) {
        return FirstInfo::isEqual(std::get<0>(lhs), std::get<0>(rhs)) &&
               SecondInfo::isEqual(std::get<1>(lhs), std::get<1>(rhs)) &&
               ThirdInfo::isEqual(std::get<2>(lhs), std::get<2>(rhs));
    }
};

template <>
struct DenseMapInfo<StringView> {
    static inline StringView getEmptyKey() {
        return StringView(
            reinterpret_cast<const char*>(~static_cast<uintptr_t>(0)), 0);
    }

    static inline StringView getTombstoneKey() {
        return StringView(
            reinterpret_cast<const char*>(~static_cast<uintptr_t>(1)), 0);
    }

    static unsigned getHashValue(const StringView& v) {
        assert(v.data() != getEmptyKey().data() &&
               "Cannot hash the empty key!");
        assert(v.data() != getTombstoneKey().data() &&
               "Cannot hash the tombstone key!");
        return (unsigned)(std::hash<StringView>()(v));
    }

    static bool isEqual(const StringView& lhs, const StringView& rhs) {
        if (rhs.data() == getEmptyKey().data())
            return lhs.data() == getEmptyKey().data();
        if (rhs.data() == getTombstoneKey().data())
            return lhs.data() == getTombstoneKey().data();
        return lhs == rhs;
    }
};

template <typename T>
struct DenseMapInfo<ArrayRef<T>> {
    static inline ArrayRef<T> getEmptyKey() {
        return ArrayRef<T>(
            reinterpret_cast<const T*>(~static_cast<uintptr_t>(0)), size_t(0));
    }
    static inline ArrayRef<T> getTombstoneKey() {
        return ArrayRef<T>(
            reinterpret_cast<const T*>(~static_cast<uintptr_t>(1)), size_t(0));
    }
    static unsigned getHashValue(ArrayRef<T> val) {
        assert(val.data() != getEmptyKey().data() &&
               "Cannot hash the empty key!");
        assert(val.data() != getTombstoneKey().data() &&
               "Cannot hash the tombstone key!");
        return (unsigned)(std::hash<ArrayRef<T>>()(val));
    }
    static bool isEqual(ArrayRef<T> lhs, ArrayRef<T> rhs) {
        if (rhs.data() == getEmptyKey().data())
            return lhs.data() == getEmptyKey().data();
        if (rhs.data() == getTombstoneKey().data())
            return lhs.data() == getTombstoneKey().data();
        return lhs == rhs;
    }
};
}
