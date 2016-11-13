#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

// This file contains various helper functions that are useful for other part of
// the DataStructure library (mostly math utilities)
// It is not designed to be exposed to the public, as indicated from the fact
// that all functions are put into a "detail" namespace. Include it at your own
// risk.

namespace ds {
namespace detail {

inline constexpr uint64_t nextPowerOfTwo(uint64_t a) {
    a |= (a >> 1);
    a |= (a >> 2);
    a |= (a >> 4);
    a |= (a >> 8);
    a |= (a >> 16);
    a |= (a >> 32);
    return a + 1;
}

inline unsigned countLeadingZeros(unsigned x) {
    unsigned n = 0;
    if (x <= 0x0000ffff)
        n += 16, x <<= 16;
    if (x <= 0x00ffffff)
        n += 8, x <<= 8;
    if (x <= 0x0fffffff)
        n += 4, x <<= 4;
    if (x <= 0x3fffffff)
        n += 2, x <<= 2;
    if (x <= 0x7fffffff)
        ++n;
    return n;
}

inline unsigned log2_32_ceil(unsigned v) {
    return 32 - countLeadingZeros(v - 1);
}

template <int p, int n>
struct maxPow2Less {
    enum { c = 2 * n < p };

    static constexpr int value = c ? (maxPow2Less<c * p, 2 * c * n>::value) : n;
};

template <>
struct maxPow2Less<0, 0> {
    static constexpr int value = 0;
};

template <typename T>
int integerLog2(T x) {
    assert(x > 0);
    constexpr int width = std::numeric_limits<T>::digits;
    int n = maxPow2Less<width, 4>::value;
    int result = 0;
    while (x != 1) {
        T t = static_cast<T>(x >> n);
        if (t) {
            result += n;
            x = t;
        }
        n /= 2;
    }
    return result;
}

template <typename T>
int lowestBit(T x) {
    assert(x >= 1);
    // clear all bits on except the rightmost one,
    // then calculate the logarithm base 2
    return integerLog2<T>(x - (x & (x - 1)));
}

template <typename T>
struct isPodLike {
    static constexpr bool value = std::is_trivially_copyable<T>::value;
};

template <typename T, typename U>
struct isPodLike<std::pair<T, U>> {
    static constexpr bool value = isPodLike<T>::value && isPodLike<U>::value;
};
}
}