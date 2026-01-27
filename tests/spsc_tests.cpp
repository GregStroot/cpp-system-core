#include <gtest/gtest.h>
#include "queues/SpscRing.h"
#include <thread>
#include <vector>
#include <atomic>

class SpscRingTest : public ::testing::Test {};

// 1. Basic Single-Threaded Logic (Sanity Check)
TEST_F(SpscRingTest, BasicPushPop) {
    My::SpscRing<int> ring(4); // Capacity 4
    int val;

    EXPECT_TRUE(ring.empty());
    EXPECT_FALSE(ring.full());

    // Fill it
    EXPECT_TRUE(ring.push(1));
    EXPECT_TRUE(ring.push(2));
    EXPECT_TRUE(ring.push(3));
    // Note: If implemented as circular with "one slot open", capacity might be size-1.
    // Adjust expectations based on your logic.

    // Pop one
    EXPECT_TRUE(ring.pop(val));
    EXPECT_EQ(val, 1);

    EXPECT_TRUE(ring.push(4));

    // Drain
    EXPECT_TRUE(ring.pop(val)); EXPECT_EQ(val, 2);
    EXPECT_TRUE(ring.pop(val)); EXPECT_EQ(val, 3);
    EXPECT_TRUE(ring.pop(val)); EXPECT_EQ(val, 4);

    EXPECT_TRUE(ring.empty());
}

TEST_F(SpscRingTest, FullBehavior) {
    My::SpscRing<int> ring(2);
    EXPECT_TRUE(ring.push(1));
    EXPECT_TRUE(ring.push(2)); // Depending on implementation, might be full here

    // Should fail if full
    // (Assuming size 2 means storage for 2 items)
    EXPECT_FALSE(ring.push(3));
}

// 2. Concurrency Stress Test (The Real Exam)
TEST_F(SpscRingTest, ProducerConsumerStress) {
    // Large capacity to test cache thrashing, or small to test wrapping?
    // Let's use small to force constant wrapping.
    const size_t ring_size = 1024;
    const int num_iterations = 1'000'000;

    My::SpscRing<int> ring(ring_size);

    std::atomic<bool> done = false;
    std::vector<int> consumed_data;
    consumed_data.reserve(num_iterations);

    // Consumer Thread
    std::thread consumer([&]() {
        int val;
        int count = 0;
        while (count < num_iterations) {
            if (ring.pop(val)) {
                consumed_data.push_back(val);
                count++;
            } else {
                // Busy wait / yield
                std::this_thread::yield();
            }
        }
    });

    // Producer Thread
    std::thread producer([&]() {
        for (int i = 0; i < num_iterations; ++i) {
            while (!ring.push(i)) {
                // Busy wait / yield
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    // Verification
    ASSERT_EQ(consumed_data.size(), num_iterations);
    for (int i = 0; i < num_iterations; ++i) {
        ASSERT_EQ(consumed_data[i], i) << "Mismatch at index " << i;
    }
}

// 3. Move Semantics Test
TEST_F(SpscRingTest, MoveSemantics) {
    My::SpscRing<std::unique_ptr<int>> ring(4);

    auto ptr = std::make_unique<int>(99);
    EXPECT_TRUE(ring.push(std::move(ptr)));
    EXPECT_EQ(ptr, nullptr); // Should have been moved from

    std::unique_ptr<int> out;
    EXPECT_TRUE(ring.pop(out));
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 99);
}
