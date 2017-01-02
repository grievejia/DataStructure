#include "DataStructure/FlatSet.h"

#include "gtest/gtest.h"

using namespace ds;

namespace {

TEST(FlatSetTest, FlatSetTest) {
    FlatSet<int> svec;
    EXPECT_TRUE(svec.empty());

    // Insert 0..9
    for (auto i = 0; i < 10; ++i) {
        auto pair = svec.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_TRUE(pair.second);
        EXPECT_EQ(svec.size(), i + 1u);
        EXPECT_TRUE(std::is_sorted(svec.begin(), svec.end()));
    }

    // Insert 0..9 again
    for (auto i = 0; i < 10; ++i) {
        auto pair = svec.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_FALSE(pair.second);
        EXPECT_EQ(svec.size(), 10u);
    }

    // Remove 0..9
    for (auto i = 0; i < 10; ++i) {
        svec.erase(i);
        EXPECT_EQ(svec.size(), 9u - i);

        EXPECT_FALSE(svec.count(i));
    }

    // Insert 9..0
    for (auto i = 9; i >= 0; --i) {
        auto pair = svec.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_TRUE(pair.second);
        EXPECT_EQ(svec.size(), 10u - i);
    }

    // Remove 0..9
    for (auto i = 0; i < 10; ++i) {
        svec.erase(i);
        EXPECT_EQ(svec.size(), 9u - i);

        EXPECT_FALSE(svec.count(i));
    }
}
}