#include "DataStructure/DynamicBitSet.h"

#include "gtest/gtest.h"

using namespace ds;

namespace {

TEST(DynamicBitSetTest, BasicTest) {
    DynamicBitSet<> s0;
    EXPECT_TRUE(s0.empty());
    EXPECT_EQ(s0.size(), 0u);
    EXPECT_EQ(s0.num_blocks(), 0u);

    DynamicBitSet<> s1(8);
    EXPECT_EQ(s1.size(), 8u);
    EXPECT_EQ(s1.num_blocks(), 1u);

    for (auto i = 0; i < 8; ++i)
        EXPECT_FALSE(s1.test(i));

    s1.set(3);
    EXPECT_TRUE(s1.test(3));
    auto s2 = s1;
    EXPECT_TRUE(s2.test(3));
    EXPECT_EQ(s1, s2);
    s2.reset();
    s2.flip(4);
    EXPECT_FALSE(s2.test(3));
    EXPECT_TRUE(s2.test(4));
    EXPECT_NE(s1, s2);

    s1 |= s2;
    EXPECT_TRUE(s1.test(3));
    EXPECT_TRUE(s1.test(4));
    EXPECT_EQ(s1.find_first(), 3);
    EXPECT_EQ(s1.find_next(3), 4);
    // Apparently EXPECT_EQ binds parameters by reference so we have to use this
    // workaround
    auto npos = DynamicBitSet<>::npos;
    EXPECT_EQ(s1.find_next(4), npos);

    DynamicBitSet<> s3(8);
    s3.flip();
    for (auto i = 0; i < 8; ++i)
        EXPECT_TRUE(s3.test(i));

    s1 &= s3;
    EXPECT_EQ(s1, DynamicBitSet<>(8, 0b00011000));
}
}
