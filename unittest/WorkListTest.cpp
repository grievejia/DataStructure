#include "DataStructure/FIFOWorkList.h"
#include "DataStructure/PriorityWorkList.h"
#include "DataStructure/UnorderedWorkList.h"

#include <algorithm>
#include <numeric>
#include <random>

#include "gtest/gtest.h"

using namespace ds;

namespace {

TEST(WorkListTest, FIFOWorkListTest) {
    auto workList = FIFOWorkList<int>();
    ASSERT_TRUE(workList.empty());

    auto constexpr testSize = 100u;
    std::array<int, testSize> arr;
    std::iota(arr.begin(), arr.end(), testSize - 50);
    std::shuffle(std::begin(arr), std::end(arr), std::default_random_engine());

    for (auto i : arr)
        workList.enqueue(i);
    for (auto i = 0u; i < testSize; ++i) {
        ASSERT_FALSE(workList.empty());
        EXPECT_EQ(workList.front(), arr[i]);
        EXPECT_EQ(workList.dequeue(), arr[i]);
    }
    EXPECT_TRUE(workList.empty());
}

TEST(WorkListTest, PriorityWorkListTest) {
    auto workList = PriorityWorkList<int>();
    ASSERT_TRUE(workList.empty());

    auto constexpr testSize = 100u;
    std::array<int, testSize> arr;
    std::iota(arr.begin(), arr.end(), testSize - 50);
    std::shuffle(std::begin(arr), std::end(arr), std::default_random_engine());

    for (auto i : arr)
        workList.enqueue(i);
    std::sort(arr.begin(), arr.end(), std::greater<int>());
    for (auto i = 0u; i < testSize; ++i) {
        ASSERT_FALSE(workList.empty());
        EXPECT_EQ(workList.front(), arr[i]);
        EXPECT_EQ(workList.dequeue(), arr[i]);
    }
    EXPECT_TRUE(workList.empty());
}

TEST(WorkListTest, UnorderedWorkListTest) {
    auto workList = UnorderedWorkList<int>();
    ASSERT_TRUE(workList.empty());

    workList.enqueue(42);
    EXPECT_EQ(workList.size(), 1u);
    EXPECT_EQ(workList.front(), 42);
    EXPECT_FALSE(workList.empty());

    workList.enqueue(43);
    EXPECT_EQ(workList.size(), 2u);
    EXPECT_EQ(workList.front(), 42);

    auto elem = workList.dequeue();
    EXPECT_EQ(elem, 42);
    EXPECT_EQ(workList.size(), 1u);
    EXPECT_EQ(workList.front(), 43);

    workList.enqueue(44);
    EXPECT_EQ(workList.size(), 2u);
    EXPECT_EQ(workList.front(), 43);

    elem = workList.dequeue();
    EXPECT_EQ(elem, 43);
    EXPECT_EQ(workList.size(), 1u);
    EXPECT_EQ(workList.front(), 44);

    elem = workList.dequeue();
    EXPECT_EQ(elem, 44);
    EXPECT_TRUE(workList.empty());
}
}
