#pragma once

#include "DataStructure/Detail.h"
#include "DataStructure/SmallVector.h"

#include <algorithm>
#include <array>
#include <vector>

namespace ds {

// This is mostly a copy of llvm::ArrayRef

// TODO: range-v3 will deprecate this?

template <typename T>
class ArrayRef {
public:
    using iterator = const T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using size_type = std::size_t;

private:
    const T* array;
    size_type length;

public:
    ArrayRef() : array(nullptr), length(0) {}
    ArrayRef(const T* array, size_type length) : array(array), length(length) {}
    ArrayRef(const T* begin, const T* end)
        : array(begin), length(end - begin) {}
    template <typename A>
    ArrayRef(const std::vector<T, A>& vec)
        : array(vec.data()), length(vec.size()) {}
    template <size_t N>
    constexpr ArrayRef(const std::array<T, N>& arr)
        : array(arr.data()), length(N) {}
    template <size_t N>
    constexpr ArrayRef(const T (&arr)[N]) : array(arr), length(N) {}
    ArrayRef(const std::initializer_list<T>& vec)
        : array(vec.begin() == vec.end() ? nullptr : vec.begin()),
          length(vec.size()) {}
    template <typename U>
    ArrayRef(const SmallVectorTemplateCommon<T, U>& vec)
        : array(vec.data()), length(vec.size()) {}

    // Construct an ArrayRef<const T*> from ArrayRef<T*>. This uses SFInAE to
    // ensure that only ArrayRefs of pointers can be converted.
    template <typename U>
    ArrayRef(
        const ArrayRef<U*>& A,
        typename std::enable_if<
            std::is_convertible<U* const*, T const*>::value>::type* = nullptr)
        : array(A.data()), length(A.size()) {}

    // Construct an ArrayRef<const T*> from a SmallVector<T*>. This is
    // templated in order to avoid instantiating SmallVectorTemplateCommon<T>
    // whenever we copy-construct an ArrayRef.
    template <typename U, typename DummyT>
    ArrayRef(
        const SmallVectorTemplateCommon<U*, DummyT>& vec,
        typename std::enable_if<
            std::is_convertible<U* const*, T const*>::value>::type* = nullptr)
        : array(vec.data()), length(vec.size()) {}

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
    ArrayRef<T> slice(size_type n) const {
        assert(n <= size() && "Invalid specifier");
        return ArrayRef<T>(data() + n, size() - n);
    }

    // slice(n, m) - Chop off the first n elements of the array, and keep m
    // elements in the array.
    ArrayRef<T> slice(size_type n, size_type m) const {
        assert(n + m <= size() && "Invalid specifier");
        return ArrayRef<T>(data() + n, m);
    }

    // Drop the first n elements of the array.
    ArrayRef<T> drop_front(size_type n = 1) const {
        assert(size() >= n && "Dropping more elements than exist");
        return slice(n, size() - n);
    }

    // Drop the last n elements of the array.
    ArrayRef<T> drop_back(size_type n = 1) const {
        assert(size() >= n && "Dropping more elements than exist");
        return slice(0, size() - n);
    }

    // Return a copy of *this with only the first n elements.
    ArrayRef<T> take_front(size_type n = 1) const {
        if (n >= size())
            return *this;
        return drop_back(size() - n);
    }

    // Return a copy of *this with only the last n elements.
    ArrayRef<T> take_back(size_type n = 1) const {
        if (n >= size())
            return *this;
        return drop_front(size() - n);
    }

    const T& operator[](size_type index) const {
        assert(index < length && "Invalid index!");
        return array[index];
    }

    std::vector<T> vec() const { return std::vector<T>(array, array + length); }
    operator std::vector<T>() const {
        return std::vector<T>(array, array + length);
    }
};

template <typename T>
class MutableArrayRef : public ArrayRef<T> {
public:
    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using typename ArrayRef<T>::size_type;

    MutableArrayRef() : ArrayRef<T>() {}
    MutableArrayRef(T* data, size_type length) : ArrayRef<T>(data, length) {}
    MutableArrayRef(T* begin, T* end) : ArrayRef<T>(begin, end) {}
    MutableArrayRef(SmallVectorImpl<T>& vec) : ArrayRef<T>(vec) {}
    MutableArrayRef(std::vector<T>& vec) : ArrayRef<T>(vec) {}
    template <size_t N>
    constexpr MutableArrayRef(std::array<T, N>& arr) : ArrayRef<T>(arr) {}
    template <size_t N>
    constexpr MutableArrayRef(T (&arr)[N]) : ArrayRef<T>(arr) {}

    T* data() const { return const_cast<T*>(ArrayRef<T>::data()); }

    iterator begin() const { return data(); }
    iterator end() const { return data() + this->size(); }

    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }

    // front - Get the first element.
    T& front() const {
        assert(!this->empty());
        return data()[0];
    }

    // back - Get the last element.
    T& back() const {
        assert(!this->empty());
        return data()[this->size() - 1];
    }

    // slice(n) - Chop off the first n elements of the array.
    MutableArrayRef<T> slice(size_type n) const {
        assert(n <= this->size() && "Invalid specifier");
        return MutableArrayRef<T>(data() + n, this->size() - n);
    }

    // slice(n, m) - Chop off the first n elements of the array, and keep m
    // elements in the array.
    MutableArrayRef<T> slice(size_type n, size_type m) const {
        assert(n + m <= this->size() && "Invalid specifier");
        return MutableArrayRef<T>(data() + n, m);
    }

    // Drop the first n elements of the array.
    MutableArrayRef<T> drop_front(size_type n = 1) const {
        assert(this->size() >= n && "Dropping more elements than exist");
        return slice(n, this->size() - n);
    }

    MutableArrayRef<T> drop_back(size_type n = 1) const {
        assert(this->size() >= n && "Dropping more elements than exist");
        return slice(0, this->size() - n);
    }

    // Return a copy of *this with only the first \p N elements.
    MutableArrayRef<T> take_front(size_type n = 1) const {
        if (n >= this->size())
            return *this;
        return drop_back(this->size() - n);
    }

    /// \brief Return a copy of *this with only the last \p N elements.
    MutableArrayRef<T> take_back(size_type n = 1) const {
        if (n >= this->size())
            return *this;
        return drop_front(this->size() - n);
    }

    T& operator[](size_type index) const {
        assert(index < this->size() && "Invalid index!");
        return data()[index];
    }
};

template <typename T>
ArrayRef<T> make_array_ref(const T& oneElt) {
    return ArrayRef<T>(std::initializer_list<T>{oneElt});
}

template <typename T>
ArrayRef<T> make_array_ref(const T* data, size_t length) {
    return ArrayRef<T>(data, length);
}

template <typename T>
ArrayRef<T> make_array_ref(const T* begin, const T* end) {
    return ArrayRef<T>(begin, end);
}

template <typename T>
ArrayRef<T> make_array_ref(const SmallVectorImpl<T>& vec) {
    return vec;
}

template <typename T, unsigned N>
ArrayRef<T> make_array_ref(const SmallVector<T, N>& vec) {
    return vec;
}

template <typename T>
ArrayRef<T> make_array_ref(const std::vector<T>& vec) {
    return vec;
}

template <typename T>
ArrayRef<T> make_array_ref(const ArrayRef<T>& vec) {
    return vec;
}

template <typename T>
ArrayRef<T>& make_array_ref(ArrayRef<T>& vec) {
    return vec;
}

template <typename T, size_t N>
ArrayRef<T> make_array_ref(const T (&arr)[N]) {
    return ArrayRef<T>(arr);
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

namespace detail {
template <typename T>
struct isPodLike<ArrayRef<T>> {
    static const bool value = true;
};
}
}

namespace std {
template <typename T>
struct hash<ds::ArrayRef<T>> {
    void hash_combine(std::size_t& seed, const T& v) const {
        seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    size_t operator()(const ds::ArrayRef<T>& a) const {
        std::size_t seed = 0;
        for (auto const& elem : a)
            hash_combine(seed, elem);
        return seed;
    }
};
}
