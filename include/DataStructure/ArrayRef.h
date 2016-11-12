#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace ds {

// This is mostly a copy of llvm::ArrayRef

// TODO: range-v3 will deprecate this?

template <typename T>
class ArrayRef {
public:
    typedef const T* iterator;
    typedef const T* const_iterator;
    typedef size_t size_type;

    typedef std::reverse_iterator<iterator> reverse_iterator;

private:
    const T* array;
    size_type length;

public:
    ArrayRef() : array(nullptr), length(0) {}
    ArrayRef(const T* array, size_t length) : array(array), length(length) {}
    template <typename A>
    ArrayRef(const std::vector<T, A>& vec)
        : array(vec.data()), length(vec.size()) {}
    template <size_t n>
    constexpr ArrayRef(const T (&Arr)[n]) : array(Arr), length(n) {}
    ArrayRef(const std::initializer_list<T>& vec)
        : array(vec.begin() == vec.end() ? nullptr : vec.begin()),
          length(vec.size()) {}

    // Construct an ArrayRef<const T*> from ArrayRef<T*>. This uses SFInAE to
    // ensure that only ArrayRefs of pointers can be converted.
    template <typename U>
    ArrayRef(
        const ArrayRef<U*>& A,
        typename std::enable_if<
            std::is_convertible<U* const*, T const*>::value>::type* = nullptr)
        : array(A.data()), length(A.size()) {}

    // Construct an ArrayRef<const T*> from std::vector<T*>. This uses SFInAE to
    // ensure that only vectors of pointers can be converted.
    template <typename U, typename A>
    ArrayRef(const std::vector<U*, A>& vec,
             typename std::enable_if<
                 std::is_convertible<U* const*, T const*>::value>::type* = 0)
        : array(vec.data()), length(vec.size()) {}

    iterator begin() const { return array; }
    iterator end() const { return array + length; }

    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }

    bool empty() const { return length == 0; }
    const T* data() const { return array; }
    size_t size() const { return length; }
    const T& front() const {
        assert(!empty());
        return array[0];
    }
    const T& back() const {
        assert(!empty());
        return array[length - 1];
    }

    bool equals(ArrayRef rhs) const {
        if (length != rhs.length)
            return false;
        return std::equal(begin(), end(), rhs.begin());
    }

    // slice(n) - Chop off the first n elements of the array.
    ArrayRef<T> slice(unsigned n) const {
        assert(n <= size() && "Invalid specifier");
        return ArrayRef<T>(data() + n, size() - n);
    }

    // slice(n, m) - Chop off the first n elements of the array, and keep m
    // elements in the array.
    ArrayRef<T> slice(unsigned n, unsigned m) const {
        assert(n + m <= size() && "Invalid specifier");
        return ArrayRef<T>(data() + n, m);
    }

    // Drop the first \p n elements of the array.
    ArrayRef<T> drop_front(unsigned n = 1) const {
        assert(size() >= n && "Dropping more elements than exist");
        return slice(n, size() - n);
    }

    // Drop the last \p n elements of the array.
    ArrayRef<T> drop_back(unsigned n = 1) const {
        assert(size() >= n && "Dropping more elements than exist");
        return slice(0, size() - n);
    }

    const T& operator[](size_t index) const {
        assert(index < length && "Invalid index!");
        return array[index];
    }

    std::vector<T> vec() const { return std::vector<T>(array, array + length); }
    operator std::vector<T>() const {
        return std::vector<T>(array, array + length);
    }
};

template <typename T>
ArrayRef<T> make_array_ref(const T* data, size_t length) {
    return ArrayRef<T>(data, length);
}

template <typename T>
ArrayRef<T> make_array_ref(const T* begin, const T* end) {
    return ArrayRef<T>(begin, end);
}

template <typename T>
ArrayRef<T> make_array_ref(const std::vector<T>& Vec) {
    return Vec;
}

template <typename T>
ArrayRef<T> make_array_ref(const ArrayRef<T>& Vec) {
    return Vec;
}

template <typename T>
ArrayRef<T>& make_array_ref(ArrayRef<T>& Vec) {
    return Vec;
}

template <typename T, size_t N>
ArrayRef<T> make_array_ref(const T (&Arr)[N]) {
    return ArrayRef<T>(Arr);
}

template <typename T>
bool operator==(ArrayRef<T> lhs, ArrayRef<T> rhs) {
    return lhs.equals(rhs);
}

template <typename T>
bool operator!=(ArrayRef<T> lhs, ArrayRef<T> rhs) {
    return !(lhs == rhs);
}

template <typename T>
bool operator<(ArrayRef<T> lhs, ArrayRef<T> rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
}

template <typename T>
bool operator<=(ArrayRef<T> lhs, ArrayRef<T> rhs) {
    return !(rhs < lhs);
}

template <typename T>
bool operator>(ArrayRef<T> lhs, ArrayRef<T> rhs) {
    return rhs < lhs;
}

template <typename T>
bool operator>=(ArrayRef<T> lhs, ArrayRef<T> rhs) {
    return !(lhs < rhs);
}
}
