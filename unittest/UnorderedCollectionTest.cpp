#include "DataStructure/UnorderedCollection.h"

#include <functional>
#include <unordered_set>

#include "gtest/gtest.h"

using namespace ds;

namespace {

TEST(UnorderedCollectionTest, BasicTest) {
    UnorderedCollection<int> c;
    EXPECT_EQ(c.size(), 0u);
    EXPECT_EQ(c.empty(), true);

    auto& i = c.create(3);
    auto& j = c.create(4);
    EXPECT_EQ(i, 3);
    EXPECT_EQ(j, 4);
    EXPECT_EQ(c.size(), 2u);
    EXPECT_EQ(c.empty(), false);

    auto c2 = std::move(c);
    EXPECT_EQ(c2.size(), 2u);
    EXPECT_EQ(i, 3);
    EXPECT_EQ(j, 4);
    c2.clear();
    EXPECT_EQ(c2.size(), 0u);
}

struct Base {
    int i;
    Base(int i) : i(i) {}
    virtual ~Base() = default;
};

struct Derived : public Base {
    int j;
    Derived(int i, int j) : Base(i), j(j) {}
};

TEST(UnorderedCollectionTest, PolyTest) {
    UnorderedCollection<Base> c;

    auto& b = c.create(1);
    EXPECT_EQ(b.i, 1);
    auto& d = c.polyCreate<Derived>(2, 3);
    EXPECT_EQ(d.i, 2);
    EXPECT_EQ(d.j, 3);
    EXPECT_EQ(c.size(), 2u);

    c.remove(b);
    EXPECT_EQ(c.size(), 1u);
    EXPECT_EQ(d.i, 2);
    EXPECT_EQ(d.j, 3);

    c.remove(d);
    EXPECT_EQ(c.size(), 0u);
}

struct LiveCounter {
    static unsigned liveCnt;
    unsigned value;

    LiveCounter(unsigned v) : value(v) { ++liveCnt; }
    LiveCounter(const LiveCounter& rhs) : value(rhs.value) { ++liveCnt; }
    LiveCounter(LiveCounter&& rhs) noexcept : value(rhs.value) { ++liveCnt; }

    ~LiveCounter() { --liveCnt; }
};

unsigned LiveCounter::liveCnt = 0u;

TEST(UnorderedCollectionTest, LifetimeTest) {
    LiveCounter::liveCnt = 0u;
    UnorderedCollection<LiveCounter> c;

    auto& e0 = c.create(1);
    auto& e1 = c.create(2);
    EXPECT_EQ(LiveCounter::liveCnt, 2u);
    EXPECT_EQ(e0.value, 1);
    EXPECT_EQ(e1.value, 2);

    UnorderedCollection<LiveCounter> c2;
    std::swap(c, c2);
    EXPECT_EQ(LiveCounter::liveCnt, 2u);

    c2.remove(e0);
    EXPECT_EQ(c2.size(), 1u);
    EXPECT_EQ(e1.value, 2);
    EXPECT_EQ(LiveCounter::liveCnt, 1u);
}

TEST(UnorderedCollectionTest, RemoveBatchTest) {
    LiveCounter::liveCnt = 0u;
    UnorderedCollection<LiveCounter> c, c2;
    auto& e0 = c.create(0);
    auto& e1 = c.create(1);
    auto& e2 = c.create(2);
    auto& e3 = c.create(3);
    auto& e4 = c.create(4);
    auto& e5 = c.create(5);

    auto& ee0 = c2.create(0);
    auto& ee1 = c2.create(1);

    EXPECT_EQ(LiveCounter::liveCnt, 8u);
    std::unordered_set<const LiveCounter*> deadSet = {&e1, &e3, &e5, &ee0,
                                                      &ee1};
    c.removeBatch(deadSet);
    EXPECT_EQ(c.size(), 3u);
    EXPECT_EQ(LiveCounter::liveCnt, 5u);
    EXPECT_EQ(e0.value, 0);
    EXPECT_EQ(e2.value, 2);
    EXPECT_EQ(e4.value, 4);

    c2.removeBatch(deadSet);
    EXPECT_EQ(c2.size(), 0u);
    EXPECT_EQ(LiveCounter::liveCnt, 3u);
}

TEST(UnorderedCollectionTest, IterationTest) {
    UnorderedCollection<int> c;
    for (int i = 0; i < 10; ++i)
        c.create(i);

    int j = 0;
    for (auto& elem : c) {
        EXPECT_EQ(elem, j);
        ++j;
    }

    [](const UnorderedCollection<int>& c) {
        int j = 0;
        for (auto const& elem : c) {
            EXPECT_EQ(elem, j);
            ++j;
        }
    }(c);
}
}