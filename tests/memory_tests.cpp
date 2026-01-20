#include<gtest/gtest.h>
#include "memory/Array.h"
using namespace My;


// ########################################################
// ##########Tests  generated automatically by Gemini######
// ########################################################
// ########################################################

// 1. Test Partial Initialization
// HFT Scenario: We created a buffer of 5, but only received 2 updates.
TEST(ArrayTest, PartialInit) {
    Array<int, 5> arr = {10, 20};

    // The first two must be correct
    EXPECT_EQ(arr[0], 10);
    EXPECT_EQ(arr[1], 20);

    // The rest is technically undefined (garbage),
    // so we don't test indices 2, 3, 4.
    EXPECT_EQ(arr.size(), 5);
}

// 2. Test Excess Initialization
// HFT Scenario: Configuration file has too many entries. Should not crash.
TEST(ArrayTest, TruncatesExcessInit) {
    // Array size 2, provided 4 elements
    Array<int, 2> arr = {1, 2, 3, 4};

    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr.size(), 2);
}

// 3. Test Fill
// HFT Scenario: Resetting a buffer between trading days.
TEST(ArrayTest, Fill) {
    Array<int, 4> arr;
    arr.fill(99);

    for(auto x : arr) {
        EXPECT_EQ(x, 99);
    }
}

// 4. Test Accessors (Front/Back)
TEST(ArrayTest, Accessors) {
    Array<int, 3> arr = {100, 200, 300};

    EXPECT_EQ(arr.front(), 100);
    EXPECT_EQ(arr.back(), 300);

    // Modify via reference
    arr.front() = 50;
    EXPECT_EQ(arr[0], 50);
}

// 5. Test Safety (Exception on out of bounds)
// Note: We only test 'at()' for exceptions. 'operator[]' would crash (assert).
TEST(ArrayTest, OutOfBoundsThrow) {
    Array<int, 3> arr = {1, 2, 3};

    EXPECT_NO_THROW(arr.at(2));
    EXPECT_THROW(arr.at(3), std::out_of_range);
}

// 6. Test Iterator Pointer Arithmetic
// HFT Scenario: Using STL algorithms on our custom array.
TEST(ArrayTest, IteratorMath) {
    Array<int, 5> arr = {10, 20, 30, 40, 50};

    auto it = arr.begin();
    EXPECT_EQ(*it, 10);

    it += 2; // Pointer arithmetic
    EXPECT_EQ(*it, 30);

    EXPECT_EQ(arr.end() - arr.begin(), 5); // Distance
}

// 7. Const Correctness
// Verify we can read from a const object
TEST(ArrayTest, ConstAccess) {
    const Array<int, 3> arr = {5, 6, 7};

    EXPECT_EQ(arr[0], 5);
    EXPECT_EQ(arr.front(), 5);
    EXPECT_EQ(arr.back(), 7);
    // arr[0] = 9; // Should fail to compile if uncommented
}
