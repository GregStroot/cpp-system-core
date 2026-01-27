#include <gtest/gtest.h>
#include "queues/CircularBuffer.h"

class CircularBufferTest : public ::testing::Test {};

TEST_F(CircularBufferTest, BasicPushPop) {
    My::CircularBuffer<int> cb(5);
    int val;

    EXPECT_TRUE(cb.empty());

    EXPECT_TRUE(cb.push(1));
    EXPECT_TRUE(cb.push(2));

    EXPECT_EQ(cb.size(), 2);

    EXPECT_TRUE(cb.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_EQ(cb.size(), 1);
}

TEST_F(CircularBufferTest, WrapAround) {
    // Capacity of 3
    My::CircularBuffer<int> cb(3);
    int val;

    // Fill it
    EXPECT_TRUE(cb.push(1));
    EXPECT_TRUE(cb.push(2));
    EXPECT_TRUE(cb.push(3));
    EXPECT_TRUE(cb.full());
    EXPECT_FALSE(cb.push(4)); // Should fail

    // Pop two to make space at the front (indices 0 and 1 free)
    EXPECT_TRUE(cb.pop(val)); // 1
    EXPECT_TRUE(cb.pop(val)); // 2

    // Push into the freed space (wrapping logic)
    EXPECT_TRUE(cb.push(4));
    EXPECT_TRUE(cb.push(5));

    EXPECT_TRUE(cb.full());

    // Verify order is FIFO: 3, 4, 5
    EXPECT_TRUE(cb.pop(val)); EXPECT_EQ(val, 3);
    EXPECT_TRUE(cb.pop(val)); EXPECT_EQ(val, 4);
    EXPECT_TRUE(cb.pop(val)); EXPECT_EQ(val, 5);
}

TEST_F(CircularBufferTest, ZeroCapacity) {
    My::CircularBuffer<int> cb(0);
    int val;
    EXPECT_FALSE(cb.push(1));
    EXPECT_FALSE(cb.pop(val));
    EXPECT_TRUE(cb.empty());
    EXPECT_TRUE(cb.full());
}

TEST_F(CircularBufferTest, MoveOnlySemantics) {
    // std::unique_ptr cannot be copied. It enforces move semantics.
    My::CircularBuffer<std::unique_ptr<int>> cb(3);

    auto p1 = std::make_unique<int>(10);
    auto p2 = std::make_unique<int>(20);

    // --- Test Push ---
    // If your push(T&&) is missing std::move(), this line will NOT compile.
    EXPECT_TRUE(cb.push(std::move(p1)));
    EXPECT_TRUE(cb.push(std::move(p2)));

    // Verify p1 was actually stolen (moved from)
    EXPECT_EQ(p1, nullptr);

    // --- Test Pop ---
    std::unique_ptr<int> out;

    // If your pop(T&) is doing 'output = buff[head]', this will NOT compile.
    // It must be 'output = std::move(buff[head])'
    EXPECT_TRUE(cb.pop(out));

    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 10);
}
