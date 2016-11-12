#include "DataStructure/StringView.h"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstring>
#include <ostream>

namespace {

static constexpr unsigned CHAR_BIT = 8;

inline char ascii_tolower(char x) {
    if (x >= 'A' && x <= 'Z')
        return x - 'A' + 'a';
    else
        return x;
}

inline int ascii_strncasecmp(const char* lhs, const char* rhs, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        auto lhc = static_cast<unsigned char>(ascii_tolower(lhs[i]));
        auto rhc = static_cast<unsigned char>(ascii_tolower(rhs[i]));
        if (lhc != rhc)
            return lhc < rhc ? -1 : 1;
    }
    return 0;
}

int compareMemory(const char* lhs, const char* rhs, size_t len) {
    if (len == 0)
        return 0;
    else
        return std::memcmp(lhs, rhs, len);
}
}

namespace ds {

const size_t StringView::npos = ~size_t(0);

StringView::StringView(const char* s) : _data(s) {
    assert(s && "StringView cannot be built from NULL");
    length = std::strlen(s);
}

StringView::StringView(const char* s, size_t len) : _data(s), length(len) {
    assert((_data || length == 0) &&
           "StringView cannot be built from NULL with non-null length");
}

bool StringView::equals(StringView rhs) const {
    return (length == rhs.length &&
            compareMemory(_data, rhs._data, rhs.length) == 0);
}

bool StringView::equals_lower(StringView rhs) const {
    return length == rhs.length && compare_lower(rhs) == 0;
}

int StringView::compare(StringView rhs) const {
    if (int res = compareMemory(_data, rhs._data, std::min(length, rhs.length)))
        return res < 0 ? -1 : 1;
    if (length == rhs.length)
        return 0;
    return length < rhs.length ? -1 : 1;
}

int StringView::compare_lower(StringView rhs) const {
    if (int res =
            ascii_strncasecmp(_data, rhs._data, std::min(length, rhs.length)))
        return res < 0 ? -1 : 1;
    if (length == rhs.length)
        return 0;
    return length < rhs.length ? -1 : 1;
}

bool StringView::startswith(StringView prefix) const {
    return length >= prefix.length &&
           compareMemory(_data, prefix._data, prefix.length) == 0;
}

bool StringView::endswith(StringView suffix) const {
    return length >= suffix.length &&
           compareMemory(end() - suffix.length, suffix._data, suffix.length) ==
               0;
}

size_t StringView::find(char c, size_t from) const {
    size_t findBegin = std::min(from, length);
    if (findBegin < length) {
        if (const void* p =
                std::memchr(_data + findBegin, c, length - findBegin))
            return static_cast<const char*>(p) - _data;
    }
    return npos;
}
size_t StringView::find(StringView s, size_t from) const {
    if (from > length)
        return npos;

    const char* needle = s.data();
    size_t n = s.size();
    if (n == 0)
        return from;

    size_t size = length - from;
    if (size < n)
        return npos;

    const char* start = _data + from;
    const char* stop = start + (size - n + 1);
    if (size < 16 || n > 255) {
        do {
            if (std::memcmp(start, needle, n) == 0)
                return start - _data;
            ++start;
        } while (start < stop);
        return npos;
    }

    uint8_t badCharSkip[256];
    std::memset(badCharSkip, n, 256);
    for (unsigned i = 0; i != n - 1; ++i)
        badCharSkip[static_cast<uint8_t>(s[i])] =
            static_cast<uint8_t>(n - 1 - i);

    do {
        if (std::memcmp(start, needle, n) == 0)
            return start - _data;
        start += badCharSkip[(uint8_t)start[n - 1]];
    } while (start < stop);

    return npos;
}
size_t StringView::rfind(char c, size_t from) const {
    from = std::min(from, length);
    size_t i = from;
    while (i != 0) {
        --i;
        if (_data[i] == c)
            return i;
    }
    return npos;
}
size_t StringView::rfind(StringView s) const {
    size_t n = s.size();
    if (n > length)
        return npos;
    for (size_t i = length - n + 1, e = 0; i != e;) {
        --i;
        if (substr(i, n).equals(s))
            return i;
    }
    return npos;
}

size_t StringView::count(char c) const {
    size_t count = 0;
    for (size_t i = 0, e = length; i != e; ++i)
        if (_data[i] == c)
            ++count;
    return count;
}

size_t StringView::count(StringView s) const {
    size_t count = 0;
    size_t n = s.size();
    if (n > length)
        return 0;
    for (size_t i = 0, e = length - n + 1; i != e; ++i)
        if (substr(i, n).equals(s))
            ++count;
    return count;
}

char StringView::front() const {
    assert(!empty());
    return _data[0];
}

char StringView::back() const {
    assert(!empty());
    return _data[length - 1];
}
char StringView::operator[](size_t idx) const {
    assert(idx < length && "Index out of bound!");
    return _data[idx];
}

StringView StringView::substr(size_t start, size_t n) const {
    start = std::min(start, length);
    return StringView(_data + start, std::min(n, length - start));
}
StringView StringView::slice(size_t start, size_t end) const {
    start = std::min(start, length);
    end = std::min(std::max(start, end), length);
    return StringView(_data + start, end - start);
}
StringView StringView::dropFront(size_t n) const {
    assert(size() >= n && "Dropping more elements than exist!");
    return substr(n);
}
StringView StringView::dropBack(size_t n) const {
    assert(size() >= n && "Dropping more elements than exist!");
    return substr(0, size() - n);
}

size_t StringView::find_first_of(char c, size_t from) const {
    return find(c, from);
}

size_t StringView::find_first_of(StringView chars, size_t from) const {
    std::bitset<1 << CHAR_BIT> charBits;
    for (size_type i = 0; i != chars.size(); ++i)
        charBits.set((unsigned char)chars[i]);

    for (size_type i = std::min(from, length), e = length; i != e; ++i)
        if (charBits.test((unsigned char)_data[i]))
            return i;
    return npos;
}

size_t StringView::find_first_not_of(char c, size_t from) const {
    for (size_type i = std::min(from, length), e = length; i != e; ++i)
        if (_data[i] != c)
            return i;
    return npos;
}

size_t StringView::find_first_not_of(StringView chars, size_t from) const {
    std::bitset<1 << CHAR_BIT> charBits;
    for (size_type i = 0; i != chars.size(); ++i)
        charBits.set((unsigned char)chars[i]);

    for (size_type i = std::min(from, length), e = length; i != e; ++i)
        if (!charBits.test((unsigned char)_data[i]))
            return i;
    return npos;
}

size_t StringView::find_last_of(char c, size_t from) const {
    return rfind(c, from);
}

size_t StringView::find_last_of(StringView chars, size_t from) const {
    std::bitset<1 << CHAR_BIT> charBits;
    for (size_type i = 0; i != chars.size(); ++i)
        charBits.set((unsigned char)chars[i]);

    for (size_type i = std::min(from, length) - 1, e = -1; i != e; --i)
        if (charBits.test((unsigned char)_data[i]))
            return i;
    return npos;
}

size_t StringView::find_last_not_of(char c, size_t from) const {
    for (size_type i = std::min(from, length) - 1, e = -1; i != e; --i)
        if (_data[i] != c)
            return i;
    return npos;
}

size_t StringView::find_last_not_of(StringView chars, size_t from) const {
    std::bitset<1 << CHAR_BIT> charBits;
    for (size_type i = 0, e = chars.size(); i != e; ++i)
        charBits.set((unsigned char)chars[i]);

    for (size_type i = std::min(from, length) - 1, e = -1; i != e; --i)
        if (!charBits.test((unsigned char)_data[i]))
            return i;
    return npos;
}

StringView StringView::ltrim(StringView chars) const {
    return dropFront(std::min(length, find_first_not_of(chars)));
}

StringView StringView::rtrim(StringView chars) const {
    return dropBack(length - std::min(length, find_last_not_of(chars) + 1));
}

StringView StringView::trim(StringView chars) const {
    return ltrim(chars).rtrim(chars);
}

std::ostream& operator<<(std::ostream& os, const StringView& s) {
    if (!s.empty()) {
        for (StringView::size_type i = 0; i < s.size(); ++i)
            os << s[i];
    }
    return os;
}

std::vector<StringView> split(StringView str, char sep, int maxSplit,
                              bool keepEmpty) {
    std::vector<StringView> ret;
    if (maxSplit > 0)
        ret.reserve(maxSplit + 1);

    while (maxSplit != 0) {
        auto idx = str.find(sep);
        if (idx == StringView::npos)
            break;

        if (keepEmpty || idx > 0)
            ret.push_back(str.slice(0, idx));

        str = str.slice(idx + 1, StringView::npos);
        --maxSplit;
    }

    if (keepEmpty || !str.empty())
        ret.push_back(str);

    return ret;
}
}
