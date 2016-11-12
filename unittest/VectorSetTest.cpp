#include "DataStructure/VectorSet.h"

#include "gtest/gtest.h"

using namespace ds;

namespace {

TEST(VectorSetTest, VectorSetTest) {
    VectorSet<int> svec;
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
    }

    // Remove 0..9
    for (auto i = 0; i < 10; ++i) {
        svec.erase(i);
        EXPECT_EQ(svec.size(), 9u - i);
        EXPECT_TRUE(std::is_sorted(svec.begin(), svec.end()));

        EXPECT_FALSE(svec.count(i));
    }

    // Insert 9..0
    for (auto i = 9; i >= 0; --i) {
        auto pair = svec.insert(i);
        EXPECT_EQ(*pair.first, i);
        EXPECT_TRUE(pair.second);
        EXPECT_EQ(svec.size(), 10u - i);
        EXPECT_TRUE(std::is_sorted(svec.begin(), svec.end()));
    }

    // Remove 0..9
    for (auto i = 0; i < 10; ++i) {
        svec.erase(i);
        EXPECT_EQ(svec.size(), 9u - i);
        EXPECT_TRUE(std::is_sorted(svec.begin(), svec.end()));

        EXPECT_FALSE(svec.count(i));
    }

    // Merge 4567 and 1234
    VectorSet<int> svec2 = {7, 4, 6, 7, 5};
    svec.assign({3, 2, 3, 1, 4});
    EXPECT_TRUE(std::is_sorted(svec.begin(), svec.end()));
    EXPECT_EQ(svec.size(), 4u);
    EXPECT_TRUE(std::is_sorted(svec2.begin(), svec2.end()));
    EXPECT_EQ(svec2.size(), 4u);
    EXPECT_TRUE(svec.merge(svec2));
    EXPECT_TRUE(std::is_sorted(svec.begin(), svec.end()));
    EXPECT_EQ(svec.size(), 7u);

    // Comparason
    EXPECT_EQ(svec, VectorSet<int>({4, 7, 3, 1, 6, 5, 2}));
    EXPECT_TRUE(svec < VectorSet<int>({8, 4, 7, 3, 1, 6, 5, 2}));
    EXPECT_TRUE(svec > VectorSet<int>({4, 3, 1, 6, 5, 2}));
}
}