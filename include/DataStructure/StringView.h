#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace ds {

class StringView {
public:
    using iterator = const char*;
    using const_iterator = const char*;
    using size_type = size_t;
    static const size_t npos;

private:
    const char* _data;
    size_t length;

public:
    StringView() : _data(nullptr), length(0) {}
    StringView(const char* s);
    StringView(const char* s, size_t len);
    StringView(const std::string& s) : _data(s.data()), length(s.length()) {}
    const char* data() const { return _data; }
    bool empty() const { return length == 0; }
    size_t size() const { return length; }
    iterator begin() const { return _data; }
    iterator end() const { return _data + length; }
    char front() const;
    char back() const;

    bool equals(StringView rhs) const;
    bool equals_lower(StringView rhs) const;

    int compare(StringView rhs) const;
    int compare_lower(StringView rhs) const;

    std::string to_string() const {
        if (!_data)
            return std::string();
        else
            return std::string(_data, length);
    }
    operator std::string() const { return to_string(); }
    char operator[](size_t idx) const;
    bool startswith(StringView prefix) const;
    bool endswith(StringView suffix) const;

    size_t find(char c, size_t from = 0) const;
    size_t find(StringView s, size_t from = 0) const;
    size_t rfind(char c, size_t from = npos) const;
    size_t rfind(StringView s) const;

    size_t find_first_of(char c, size_t from = 0) const;
    size_t find_first_of(StringView chars, size_t from = 0) const;
    size_t find_first_not_of(char c, size_t from = 0) const;
    size_t find_first_not_of(StringView chars, size_t from = 0) const;
    size_t find_last_of(char c, size_t from = npos) const;
    size_t find_last_of(StringView chars, size_t from = npos) const;
    size_t find_last_not_of(char c, size_t from = npos) const;
    size_t find_last_not_of(StringView chars, size_t from = npos) const;

    size_t count(char c) const;
    size_t count(StringView s) const;

    StringView substr(size_t start, size_t n = npos) const;
    StringView slice(size_t start, size_t end) const;
    StringView dropFront(size_t n = 1) const;
    StringView dropBack(size_t n = 1) const;
    StringView ltrim(StringView chars = " \t\n\v\f\r") const;
    StringView rtrim(StringView chars = " \t\n\v\f\r") const;
    StringView trim(StringView chars = " \t\n\v\f\r") const;
};

inline bool operator==(StringView lhs, StringView rhs) {
    return lhs.equals(rhs);
}
inline bool operator!=(StringView lhs, StringView rhs) {
    return !(lhs == rhs);
}
inline bool operator<(StringView lhs, StringView rhs) {
    return lhs.compare(rhs) == -1;
}
inline bool operator<=(StringView lhs, StringView rhs) {
    return lhs.compare(rhs) != 1;
}
inline bool operator>(StringView lhs, StringView rhs) {
    return lhs.compare(rhs) == 1;
}
inline bool operator>=(StringView lhs, StringView rhs) {
    return lhs.compare(rhs) != -1;
}

std::ostream& operator<<(std::ostream&, const StringView&);

std::vector<StringView> split(StringView str, char sep, int maxSplit = -1,
                              bool keepEmpty = true);
}

namespace std {
template <>
struct hash<ds::StringView> {
    size_t operator()(const ds::StringView& s) const {
        size_t hash = 14695981039346656037UL;
        for (size_t i = 0ul, e = s.size(); i != e; ++i) {
            hash ^= s[i];
            hash *= 1099511628211UL;
        }
        return hash;
    }
};
}
