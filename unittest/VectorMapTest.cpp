#include "DataStructure/VectorMap.h"

#include <map>

#include "gtest/gtest.h"

using namespace ds;

namespace {

struct Key {
    Key(int i = 0) {
        ++createdObjects;

        array.push_back(i);

        for (int i = 1; i < 4; ++i) {
            array.push_back(rand());
        }
    }

    Key(Key const& other) : array(other.array) {
        ++createdObjects;
        ++copies;
    }

    ~Key() { ++destroyedObjects; }

    Key(Key&& other) noexcept : array(std::move(other.array)) {
        ++createdObjects;
        ++moves;
    }

    Key& operator=(Key const& other) {
        ++copies;

        Key temp(other);
        this->swap(temp);

        return *this;
    }

    Key& operator=(Key&& other) {
        ++moves;

        array = std::move(other.array);

        return *this;
    }

    bool operator==(Key const& other) const {
        return array[0] == other.array[0];
    }

    bool operator!=(Key const& other) const { return !operator==(other); }

    bool operator<(Key const& other) const { return array[0] < other.array[0]; }

    void swap(Key& other) { array.swap(other.array); }

    std::vector<int> array;

    static unsigned createdObjects;
    static unsigned destroyedObjects;

    static unsigned copies;
    static unsigned moves;
};

unsigned Key::createdObjects = 0;

unsigned Key::destroyedObjects = 0;

unsigned Key::copies = 0;

unsigned Key::moves = 0;

struct Value {
    Value(int i = 0, std::string const& text = "") {
        ++createdObjects;

        array.push_back(i);
        this->text = text;

        for (int i = 1; i < 4; ++i) {
            array.push_back(rand());
        }
    }

    Value(Value const& other) : array(other.array), text(other.text) {
        ++createdObjects;
        ++copies;
    }

    ~Value() { ++destroyedObjects; }

    Value(Value&& other) noexcept
        : array(std::move(other.array)), text(other.text) {
        ++createdObjects;
        ++moves;
    }

    Value& operator=(Value const& other) {
        ++copies;

        Value temp(other);
        this->swap(temp);

        return *this;
    }

    Value& operator=(Value&& other) {
        ++moves;

        array = std::move(other.array);
        text = other.text;

        return *this;
    }

    bool operator==(Value const& other) const {
        return array[0] == other.array[0] && text == other.text;
    }

    bool operator!=(Value const& other) const { return !operator==(other); }

    bool operator<(Value const& other) const {
        return array[0] < other.array[0];
    }

    void swap(Value& other) { array.swap(other.array); }

    std::vector<int> array;
    std::string text;

    static unsigned createdObjects;
    static unsigned destroyedObjects;

    static unsigned copies;
    static unsigned moves;
};

unsigned Value::createdObjects = 0;

unsigned Value::destroyedObjects = 0;

unsigned Value::copies = 0;

unsigned Value::moves = 0;

template <typename _T>
struct MyAllocator {
    typedef _T value_type;

    typedef _T* pointer;
    typedef _T const* const_pointer;

    typedef _T& reference;
    typedef _T const& const_reference;

    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    MyAllocator() {}

    MyAllocator(MyAllocator const& other) : _alloc(other._alloc) {}

    template <typename __T>
    MyAllocator(MyAllocator<__T> const& other) : _alloc(other._alloc) {}

    pointer allocate(unsigned n, const_pointer* hint = 0) {
        totalMemory += n * sizeof(value_type);
        notFreedMemory += n * sizeof(value_type);

        return _alloc.allocate(n, hint);
    }

    void deallocate(pointer p, unsigned n) {
        notFreedMemory -= n * sizeof(value_type);

        _alloc.deallocate(p, n);
    }

    void construct(pointer p, value_type const& v) { _alloc.construct(p, v); }

    void construct(pointer p, value_type&& v) {
        _alloc.construct(p, std::move(v));
    }

    void destroy(pointer p) { _alloc.destroy(p); }

    unsigned max_size() const throw() { return _alloc.max_size(); }

    template <typename __T>
    struct rebind {
        typedef MyAllocator<__T> other;
    };

    static int totalMemory;
    static int notFreedMemory;

    std::allocator<value_type> _alloc;

    template <typename U>
    friend bool operator==(const MyAllocator<U>& lhs,
                           const MyAllocator<U>& rhs);
    template <typename U>
    friend bool operator!=(const MyAllocator<U>& lhs,
                           const MyAllocator<U>& rhs);
};

template <typename T>
bool operator==(const MyAllocator<T>& lhs, const MyAllocator<T>& rhs) {
    return lhs._alloc == rhs._alloc;
}

template <typename T>
bool operator!=(const MyAllocator<T>& lhs, const MyAllocator<T>& rhs) {
    return !(lhs == rhs);
}

template <typename _T>
int MyAllocator<_T>::totalMemory = 0;

template <typename _T>
int MyAllocator<_T>::notFreedMemory = 0;

auto valueRange = [](int first, int last, int step = 1) {
    std::vector<std::pair<Key, Value>> ret;
    for (auto i = first; i < last; i += step)
        ret.emplace_back(i, i);
    return ret;
};

auto keyRange = [](int first, int last, int step = 1) {
    std::vector<Key> ret;
    for (auto i = first; i < last; i += step)
        ret.emplace_back(i);
    return ret;
};

auto pairEqual = [](auto const& p0, auto const& p1) {
    return p0.first == p1.first && p0.second == p1.second;
};

template <typename KeyType, typename ValueType>
class TestCase {
private:
    std::map<KeyType, ValueType> stdMap;
    VectorMap<KeyType, ValueType> vecMap;

    template <typename SPair, typename VPair>
    static void expectPairEqual(const SPair& p0, const VPair& p1) {
        EXPECT_TRUE(pairEqual(p0, p1));
    }

    template <typename StdMapItr, typename VecMapItr>
    void expectIteratorEqual(const StdMapItr& sItr, const VecMapItr& vItr) {
        EXPECT_EQ(std::distance(stdMap.begin(), sItr),
                  std::distance(vecMap.begin(), vItr));
        EXPECT_EQ(std::distance(sItr, stdMap.end()),
                  std::distance(vItr, vecMap.end()));
    }

    void expectMapEqual() {
        EXPECT_TRUE(std::equal(stdMap.begin(), stdMap.end(), vecMap.begin(),
                               pairEqual));
        EXPECT_TRUE(std::equal(stdMap.cbegin(), stdMap.cend(), vecMap.cbegin(),
                               pairEqual));
        EXPECT_TRUE(std::equal(stdMap.rbegin(), stdMap.rend(), vecMap.rbegin(),
                               pairEqual));
        EXPECT_TRUE(std::equal(stdMap.crbegin(), stdMap.crend(),
                               vecMap.crbegin(), pairEqual));
    }

public:
    TestCase() = default;

    TestCase& insert(const KeyType& k, const ValueType& v) {
        auto sRes = stdMap.insert(std::make_pair(k, v));
        auto vRes = vecMap.insert(std::make_pair(k, v));

        expectPairEqual(*sRes.first, *vRes.first);
        EXPECT_EQ(sRes.second, vRes.second);
        expectIteratorEqual(sRes.first, vRes.first);
        expectMapEqual();

        return *this;
    }

    template <typename Container>
    TestCase& insert(const Container& c) {
        for (auto const& pair : c)
            insert(pair.first, pair.second);
        return *this;
    }

    TestCase& erase(const KeyType& k) {
        EXPECT_EQ(stdMap.erase(k), vecMap.erase(k));
        expectMapEqual();
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& erase(const Container& c) {
        for (auto const& k : c)
            erase(k);
        return *this;
    }

    TestCase& find(const KeyType& k) {
        auto sRes = stdMap.find(k);
        auto vRes = vecMap.find(k);

        expectIteratorEqual(sRes, vRes);
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& find(const Container& c) {
        for (auto const& k : c)
            find(k);
        return *this;
    }

    TestCase& count(const KeyType& k) {
        EXPECT_EQ(stdMap.count(k), vecMap.count(k));
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& count(const Container& c) {
        for (auto const& k : c)
            count(k);
        return *this;
    }

    TestCase& lower_bound(const KeyType& k) {
        auto sRes = stdMap.lower_bound(k);
        auto vRes = vecMap.lower_bound(k);

        expectIteratorEqual(sRes, vRes);
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& lower_bound(const Container& c) {
        for (auto const& k : c)
            lower_bound(k);
        return *this;
    }

    TestCase& upper_bound(const KeyType& k) {
        auto sRes = stdMap.upper_bound(k);
        auto vRes = vecMap.upper_bound(k);

        expectIteratorEqual(sRes, vRes);
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& upper_bound(const Container& c) {
        for (auto const& k : c)
            upper_bound(k);
        return *this;
    }

    TestCase& equal_range(const KeyType& k) {
        auto sRes = stdMap.equal_range(k);
        auto vRes = vecMap.equal_range(k);

        expectIteratorEqual(sRes.first, vRes.first);
        expectIteratorEqual(sRes.second, vRes.second);
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& equal_range(const Container& c) {
        for (auto const& k : c)
            equal_range(k);
        return *this;
    }

    TestCase& get(const KeyType& k) {
        EXPECT_EQ(stdMap[k], vecMap[k]);
        expectMapEqual();
        return *this;
    }

    template <typename Container,
              typename = decltype(std::begin(std::declval<Container>()),
                                  std::end(std::declval<Container>()))>
    TestCase& get(const Container& c) {
        for (auto const& k : c)
            get(k);
        return *this;
    }

    TestCase& put(const KeyType& k, const ValueType& v) {
        stdMap[k] = v;
        vecMap[k] = v;
        expectMapEqual();
        return *this;
    }

    template <typename Container>
    TestCase& put(const Container& c) {
        for (auto const& pair : c)
            put(pair.first, pair.second);
        return *this;
    }
};

TEST(VectorMapTest, ConstructorTest) {
    using Vector = std::vector<std::pair<std::string, int>>;

    Vector v;
    v.push_back(std::make_pair("1", 1));
    v.push_back(std::make_pair("2", 2));
    v.push_back(std::make_pair("3", 3));
    v.push_back(std::make_pair("4", 4));
    v.push_back(std::make_pair("5", 5));

    using VMap = VectorMap<std::string, int>;

    VMap av(v.begin(), v.end());

    EXPECT_EQ(av.size(), 5u);

    VMap::const_iterator found;

    found = av.find("1");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "1");
    EXPECT_EQ(found->second, 1);

    found = av.find("2");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "2");
    EXPECT_EQ(found->second, 2);

    found = av.find("3");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "3");
    EXPECT_EQ(found->second, 3);

    found = av.find("4");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "4");
    EXPECT_EQ(found->second, 4);

    found = av.find("5");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "5");
    EXPECT_EQ(found->second, 5);
}

TEST(VectorMapTest, CopyMoveConstructorTest) {
    using VMap = VectorMap<std::string, int>;

    auto vMap = VMap();

    vMap["a"] = 1;
    vMap["b"] = 2;
    vMap["c"] = 3;
    vMap["d"] = 4;
    vMap["e"] = 5;

    auto vMap2 = vMap;
    EXPECT_EQ(vMap, vMap2);

    auto vMap3 = std::move(vMap2);
    EXPECT_EQ(vMap, vMap3);
}

TEST(VectorMapTest, InitListConstructorTest) {
    using VMap = VectorMap<std::string, int>;

    VMap av = {{"1", 1}, {"4", 4}, {"2", 2}, {"5", 5}, {"3", 3}, {"2", 2}};

    EXPECT_EQ(av.size(), 5u);

    VMap::const_iterator found;

    found = av.find("1");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "1");
    EXPECT_EQ(found->second, 1);

    found = av.find("2");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "2");
    EXPECT_EQ(found->second, 2);

    found = av.find("3");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "3");
    EXPECT_EQ(found->second, 3);

    found = av.find("4");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "4");
    EXPECT_EQ(found->second, 4);

    found = av.find("5");
    EXPECT_NE(found, av.end());
    EXPECT_EQ(found->first, "5");
    EXPECT_EQ(found->second, 5);
}

TEST(VectorMapTest, AssignOperatorTest) {
    using VMap = VectorMap<std::string, int>;

    auto vMap = VMap();

    vMap["a"] = 1;
    vMap["b"] = 2;
    vMap["c"] = 3;
    vMap["d"] = 4;
    vMap["e"] = 5;

    auto vMap2 = VMap();
    vMap2 = vMap;
    EXPECT_EQ(vMap, vMap2);

    auto vMap3 = VMap();
    vMap3 = std::move(vMap2);
    EXPECT_EQ(vMap, vMap3);
}

TEST(VectorMapTest, ClearTest) {
    using VMap = VectorMap<std::string, int>;

    auto vMap = VMap();

    vMap["a"] = 1;
    vMap["b"] = 2;
    vMap["c"] = 3;
    vMap["d"] = 4;
    vMap["e"] = 5;

    EXPECT_EQ(vMap.erase("a"), 1u);
    EXPECT_EQ(vMap.size(), 4u);
    vMap.clear();
    EXPECT_TRUE(vMap.empty());
    EXPECT_EQ(vMap.size(), 0u);
}

TEST(VectorMapTest, EmptyContainerTest) {
    {
        VectorMap<Key, Value> vMap;
        EXPECT_TRUE(vMap.empty());
        EXPECT_EQ(vMap.size(), 0u);
        EXPECT_EQ(vMap.begin(), vMap.end());
        EXPECT_EQ(vMap.cbegin(), vMap.cend());
        EXPECT_EQ(vMap.rbegin(), vMap.rend());
        EXPECT_EQ(vMap.crbegin(), vMap.crend());
    }

    {
        std::allocator<std::pair<Key, Value>> allocator;
        VectorMap<Key, Value> vMap(allocator);
        EXPECT_TRUE(vMap.empty());
        EXPECT_EQ(vMap.size(), 0u);
        EXPECT_EQ(vMap.begin(), vMap.end());
        EXPECT_EQ(vMap.cbegin(), vMap.cend());
        EXPECT_EQ(vMap.rbegin(), vMap.rend());
        EXPECT_EQ(vMap.crbegin(), vMap.crend());
    }

    {
        std::less<Key> cmp;
        std::allocator<std::pair<Key, Value>> allocator;
        VectorMap<Key, Value> vMap(cmp, allocator);
        EXPECT_TRUE(vMap.empty());
        EXPECT_EQ(vMap.size(), 0u);
        EXPECT_EQ(vMap.begin(), vMap.end());
        EXPECT_EQ(vMap.cbegin(), vMap.cend());
        EXPECT_EQ(vMap.rbegin(), vMap.rend());
        EXPECT_EQ(vMap.crbegin(), vMap.crend());
    }
}

TEST(VectorMapTest, IncreasingInsertTest) {
    std::vector<std::pair<int, bool>> pairs;
    for (auto i = 0; i < 32; ++i)
        pairs.emplace_back(i, i % 2 == 0);
    TestCase<int, bool>().insert(pairs);

    TestCase<Key, Value>().insert(valueRange(0, 32));
}

TEST(VectorMapTest, DecreasingInsertTest) {
    std::vector<std::pair<int, bool>> pairs;
    for (auto i = 0; i < 32; ++i)
        pairs.emplace_back(32 - i, i % 2 == 0);
    TestCase<int, bool>().insert(pairs);

    TestCase<Key, Value>().insert(valueRange(32, 0, -1));
}

TEST(VectorMapTest, RandomInsertTest) {
    std::vector<int> randomNums = {10, 9,  8,  16, 17, 17, 18, 7,  7,  6,  4,
                                   3,  11, 12, 13, 2,  1,  0,  14, 15, 19, 5};

    std::vector<std::pair<int, bool>> pairs;
    for (auto i : randomNums)
        pairs.emplace_back(i, i % 2 == 0);
    TestCase<int, bool>().insert(pairs);

    std::vector<std::pair<Key, Value>> pairs2;
    for (auto i : randomNums)
        pairs2.emplace_back(32 - i, i);
    TestCase<Key, Value>().insert(pairs2);
}

TEST(VectorMapTest, IncreasingEraseTest) {
    std::vector<std::pair<int, bool>> pairs;
    std::vector<int> rmKeys;
    for (auto i = 0; i < 32; ++i) {
        pairs.emplace_back(i, i % 2 == 0);
        rmKeys.emplace_back(i);
    }
    TestCase<int, bool>().insert(pairs).erase(rmKeys);

    TestCase<Key, Value>().insert(valueRange(0, 32)).erase(keyRange(0, 32));
}

TEST(VectorMapTest, DecreasingEraseTest) {
    std::vector<std::pair<int, bool>> pairs;
    std::vector<int> rmKeys;
    for (auto i = 0; i < 32; ++i) {
        pairs.emplace_back(32 - i, i % 2 == 0);
        rmKeys.emplace_back(i);
    }
    TestCase<int, bool>().insert(pairs).erase(rmKeys);

    TestCase<Key, Value>()
        .insert(valueRange(32, 0, -1))
        .erase(keyRange(32, 0, -1));
}

TEST(VectorMapTest, RandomEraseTest) {
    std::vector<int> randomNums = {10, 9,  8,  16, 17, 17, 18, 7,  7,  6,  4,
                                   3,  11, 12, 13, 2,  1,  0,  14, 15, 19, 5};

    std::vector<std::pair<int, bool>> pairs;
    for (auto i : randomNums)
        pairs.emplace_back(i, i % 2 == 0);
    TestCase<int, bool>().insert(pairs).erase(randomNums);

    std::vector<std::pair<Key, Value>> pairs2;
    std::vector<Key> rmKeys(randomNums.begin(), randomNums.end());
    for (auto i : randomNums)
        pairs2.emplace_back(32 - i, i);
    TestCase<Key, Value>().insert(pairs2).erase(rmKeys);
}

TEST(VectorMapTest, InsertAndInsertTest) {
    std::vector<std::pair<Key, Value>> pairs = {{1, 11},  {2, 22},  {3, 33},
                                                {4, 44},  {1, 111}, {2, 222},
                                                {3, 333}, {4, 444}};
    TestCase<Key, Value>().insert(pairs);
}

TEST(VectorMapTest, InsertAndEraseTest) {
    TestCase<Key, Value>()
        .insert(valueRange(0, 50))
        .erase(keyRange(40, 45))
        .erase(keyRange(35, 50))
        .erase(keyRange(5, 15))
        .erase(keyRange(0, 20))
        .erase(keyRange(25, 30))
        .erase(keyRange(0, 50));
}

TEST(VectorMapTest, InsertAndEraseAndInsertTest) {
    TestCase<Key, Value>()
        .insert(valueRange(0, 50, 2))
        .insert(valueRange(1, 51, 2))
        .erase(keyRange(30, 40, 2))
        .erase(keyRange(31, 41, 2))
        .insert(valueRange(30, 40, 2))
        .insert(valueRange(31, 41, 2))
        .erase(keyRange(10, 20, 2))
        .erase(keyRange(11, 21, 2))
        .insert(valueRange(10, 20, 2))
        .insert(valueRange(11, 21, 2));
}

TEST(VectorMapTest, FindTest) {
    TestCase<Key, Value>()
        .insert(valueRange(0, 32, 2))
        .find(keyRange(0, 32, 2))
        .find(keyRange(1, 33, 2));
}

TEST(VectorMapTest, FindAndEraseAndFindTest) {
    TestCase<Key, Value>()
        .insert(valueRange(0, 32, 2))
        .erase(31)
        .find(31)
        .erase(1)
        .find(1)
        .erase(keyRange(15, 25))
        .find(keyRange(15, 25));
}

TEST(VectorMapTest, FindAndEraseAndFindAndInsertTest) {
    TestCase<Key, Value>()
        .insert(valueRange(0, 30, 2))
        .find(keyRange(0, 10))
        .erase(keyRange(0, 10))
        .insert(valueRange(0, 10))
        .find(keyRange(0, 10))
        .find(keyRange(10, 20))
        .erase(keyRange(10, 20))
        .insert(valueRange(10, 20))
        .find(keyRange(10, 20))
        .find(keyRange(20, 30))
        .erase(keyRange(20, 30))
        .insert(valueRange(20, 30))
        .find(keyRange(20, 30));
}

TEST(VectorMapTest, CountTest) {
    TestCase<Key, Value>()
        .insert(valueRange(0, 30, 2))
        .count(keyRange(0, 10))
        .erase(keyRange(0, 10))
        .insert(valueRange(0, 10))
        .count(keyRange(0, 10))
        .count(keyRange(10, 20))
        .erase(keyRange(10, 20))
        .insert(valueRange(10, 20))
        .count(keyRange(10, 20))
        .count(keyRange(20, 30))
        .erase(keyRange(20, 30))
        .insert(valueRange(20, 30))
        .count(keyRange(20, 30));
}

TEST(VectorMapTest, BoundTest) {
    TestCase<Key, Value>()
        .insert(valueRange(30, 10, -1))
        .erase(11)
        .erase(20)
        .erase(28)
        .lower_bound(keyRange(0, 40, 2))
        .lower_bound(keyRange(1, 41, 2))
        .upper_bound(keyRange(0, 40, 2))
        .upper_bound(keyRange(1, 41, 2))
        .equal_range(keyRange(0, 40, 2))
        .equal_range(keyRange(1, 41, 2));
}

TEST(VectorMapTest, BracketOperatorTest) {
    TestCase<Key, Value>()
        .put(1, 1)
        .put(1, 11)
        .put(1, 111)
        .get(2)
        .insert(3, 33)
        .get(3)
        .erase(3)
        .get(3);
}

TEST(VectorMapTest, MemLeakRandomTest) {
    using Allocator = MyAllocator<std::pair<Key, Value>>;

    Allocator::notFreedMemory = 0;
    Allocator::totalMemory = 0;

    Key::createdObjects = 0;
    Key::destroyedObjects = 0;

    Key::moves = 0;
    Key::copies = 0;

    Value::createdObjects = 0;
    Value::destroyedObjects = 0;

    Value::moves = 0;
    Value::copies = 0;

    {
        unsigned const maxKeyValue = 512;
        VectorMap<Key, Value, std::less<Key>, Allocator> av;

        for (int i = 0; i < 1024; ++i) {
            int const operation = rand() % 5;

            switch (operation) {
                case 0:
                    av.insert(std::make_pair(rand() % maxKeyValue, Value()));
                    break;

                case 1:
                    av.find(rand() % maxKeyValue);
                    break;

                case 2:
                    av.erase(rand() % maxKeyValue);
                    break;

                case 3: {
                    auto foundAV = av.find(rand() % maxKeyValue);

                    if (foundAV != av.end()) {
                        av.erase(foundAV);
                    }
                }

                break;

                case 4:
                    av[rand() % maxKeyValue] = Value();
                    break;
            }
        }
    }

    EXPECT_EQ(Allocator::notFreedMemory, 0);
    EXPECT_EQ(Key::createdObjects, Key::destroyedObjects);
    EXPECT_EQ(Value::createdObjects, Value::destroyedObjects);
}

TEST(VectorMapTest, MemLeakDestructorTest) {
    using Allocator = MyAllocator<std::pair<Key, Value>>;

    Allocator::notFreedMemory = 0;
    Allocator::totalMemory = 0;

    Value::createdObjects = 0;
    Value::destroyedObjects = 0;

    {
        VectorMap<int, Value, std::less<int>, Allocator> vMap;

        for (int i = 0; i < 1024; ++i)
            vMap.insert(std::make_pair(i, Value()));
    }

    EXPECT_EQ(Allocator::notFreedMemory, 0);
    EXPECT_EQ(Value::createdObjects, Value::destroyedObjects);
}

TEST(VectorMapTest, MemLeakClearTest) {
    using Allocator = MyAllocator<std::pair<Key, Value>>;

    Allocator::notFreedMemory = 0;
    Allocator::totalMemory = 0;

    Value::createdObjects = 0;
    Value::destroyedObjects = 0;

    VectorMap<int, Value, std::less<int>, Allocator> vMap;

    for (int i = 0; i < 1024; ++i)
        vMap.insert(std::make_pair(i, Value()));
    vMap.clear();

    EXPECT_EQ(Allocator::notFreedMemory, 0);
    EXPECT_EQ(Value::createdObjects, Value::destroyedObjects);
}

TEST(VectorMapTest, MemLeakCopyCtorTest) {
    using Allocator = MyAllocator<std::pair<Key, Value>>;

    Allocator::notFreedMemory = 0;
    Allocator::totalMemory = 0;

    Value::createdObjects = 0;
    Value::destroyedObjects = 0;

    {
        VectorMap<int, Value, std::less<int>, Allocator> vMap;

        for (int i = 0; i < 1024; ++i)
            vMap.insert(std::make_pair(i, Value()));

        decltype(vMap) vMap2 = vMap;
    }

    EXPECT_EQ(Allocator::notFreedMemory, 0);
    EXPECT_EQ(Value::createdObjects, Value::destroyedObjects);
}

TEST(VectorMapTest, MemLeakAssignOpTest) {
    using Allocator = MyAllocator<std::pair<Key, Value>>;

    Allocator::notFreedMemory = 0;
    Allocator::totalMemory = 0;

    Value::createdObjects = 0;
    Value::destroyedObjects = 0;

    {
        VectorMap<int, Value, std::less<int>, Allocator> vMap;

        for (int i = 0; i < 1024; ++i)
            vMap.insert(std::make_pair(i, Value()));

        VectorMap<int, Value, std::less<int>, Allocator> vMap2;
        vMap2 = vMap;
    }

    EXPECT_EQ(Allocator::notFreedMemory, 0);
    EXPECT_EQ(Value::createdObjects, Value::destroyedObjects);
}

TEST(VectorMapTest, MoveTest1) {
    using Allocator = MyAllocator<std::pair<Key, Value>>;

    Allocator::notFreedMemory = 0;
    Allocator::totalMemory = 0;

    Key::createdObjects = 0;
    Key::destroyedObjects = 0;

    Key::moves = 0;
    Key::copies = 0;

    Value::createdObjects = 0;
    Value::destroyedObjects = 0;

    Value::moves = 0;
    Value::copies = 0;

    {
        VectorMap<int, Value, std::less<int>, Allocator> vMap;
        {
            for (int i = 0; i < 1024; ++i)
                vMap.try_emplace(i, Value());
            for (int i = 0; i < 1024; ++i)
                vMap.insert_or_assign(i, Value());
        }
        EXPECT_EQ(Value::copies, 0u);
        auto vMap2 = std::move(vMap);
        EXPECT_EQ(Value::copies, 0u);
    }

    EXPECT_EQ(Allocator::notFreedMemory, 0);
    EXPECT_EQ(Key::createdObjects, Key::destroyedObjects);
    EXPECT_EQ(Value::createdObjects, Value::destroyedObjects);
}

TEST(VectorMapTest, NewInterfaceTest) {
    VectorMap<Key, Value> vMap;

    try {
        decltype(vMap)::iterator itr;
        bool changed;

        std::tie(itr, changed) = vMap.insert_or_assign(1, 11);
        EXPECT_EQ(vMap.at(1), 11);
        EXPECT_EQ(itr->first, 1);
        EXPECT_EQ(itr->second, 11);
        EXPECT_EQ(changed, true);

        std::tie(itr, changed) = vMap.try_emplace(2, 22);
        EXPECT_EQ(vMap.at(2), 22);
        EXPECT_EQ(itr->first, 2);
        EXPECT_EQ(itr->second, 22);
        EXPECT_EQ(changed, true);

        std::tie(itr, changed) = vMap.try_emplace(1, 111);
        EXPECT_EQ(vMap.at(1), 11);
        EXPECT_EQ(itr->first, 1);
        EXPECT_EQ(itr->second, 11);
        EXPECT_EQ(changed, false);

        std::tie(itr, changed) = vMap.insert_or_assign(2, 222);
        EXPECT_EQ(vMap.at(2), 222);
        EXPECT_EQ(itr->first, 2);
        EXPECT_EQ(itr->second, 222);
        EXPECT_EQ(changed, false);
    } catch (std::out_of_range&) {
        EXPECT_TRUE(false);
    }

    try {
        vMap.at(3);
        EXPECT_TRUE(false);
    } catch (std::out_of_range&) {
    } catch (...) {
        EXPECT_TRUE(false);
    }
}
}