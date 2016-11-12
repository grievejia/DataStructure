#include "DataStructure/DenseSet.h"

#include "gtest/gtest.h"

using namespace ds;

namespace {

// Test fixture
class DenseSetTest : public testing::Test {};

// Test hashing with a set of only two entries.
TEST_F(DenseSetTest, DoubleEntrySetTest) {
    DenseSet<unsigned> set(2);
    set.insert(0);
    set.insert(1);
    // Original failure was an infinite loop in this call:
    EXPECT_EQ(0u, set.count(2));
}

struct TestDenseSetInfo {
    static inline unsigned getEmptyKey() { return ~0; }
    static inline unsigned getTombstoneKey() { return ~0U - 1; }
    static unsigned getHashValue(const unsigned& Val) { return Val * 37U; }
    static bool isEqual(const unsigned& lhs, const unsigned& rhs) {
        return lhs == rhs;
    }
};

TEST_F(DenseSetTest, FindTest) {
    DenseSet<unsigned, TestDenseSetInfo> set;
    set.insert(0);
    set.insert(1);
    set.insert(2);

    // Size tests
    EXPECT_EQ(3u, set.size());

    // Normal lookup tests
    EXPECT_EQ(1u, set.count(1));
    EXPECT_EQ(0u, *set.find(0));
    EXPECT_EQ(1u, *set.find(1));
    EXPECT_EQ(2u, *set.find(2));
    EXPECT_TRUE(set.find(3) == set.end());
}

TEST_F(DenseSetTest, CustomTest) {
    DenseSet<unsigned> s;
    EXPECT_TRUE(s.empty());

    // Insert 0..9
    for (unsigned i = 0; i < 10; ++i) {
        auto pair = s.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_TRUE(pair.second);
        EXPECT_EQ(s.size(), i + 1);
    }

    // Insert 0..9 again
    for (unsigned i = 0; i < 10; ++i) {
        auto pair = s.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_FALSE(pair.second);
    }

    // Remove 0..9
    for (unsigned i = 0; i < 10; ++i) {
        s.erase(i);
        EXPECT_EQ(s.size(), 9 - i);
        EXPECT_FALSE(s.count(i));
    }

    // Insert 9..0
    for (unsigned i = 9; i != 0; --i) {
        auto pair = s.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_TRUE(pair.second);
        EXPECT_EQ(s.size(), 10 - i);
    }

    // Remove 0..9
    for (unsigned i = 0; i < 10; ++i) {
        s.erase(i);
        EXPECT_EQ(s.size(), 9 - i);

        EXPECT_FALSE(s.count(i));
    }
}
}