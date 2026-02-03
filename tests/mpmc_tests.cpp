#include <gtest/gtest.h>
#include "queues/ProducerConsumer.h"
#include <thread>
#include <vector>
#include <atomic>

class MPMCTest : public ::testing::Test {};

TEST_F(MPMCTest, BasicFlow) {
    My::ProducerConsumerQueue<int> q(10);
    q.push(42);
    EXPECT_EQ(q.pop(), 42);
}

TEST_F(MPMCTest, BlockingBehavior) {
    My::ProducerConsumerQueue<int> q(2); // Tiny capacity

    // Fill it
    q.push(1);
    q.push(2);

    std::atomic<bool> pushed_3{false};
    std::thread t([&]() {
        q.push(3); // Should block here
        pushed_3 = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(pushed_3); // Verify it blocked

    EXPECT_EQ(q.pop(), 1); // Make room
    t.join();
    EXPECT_TRUE(pushed_3); // Now it should have succeeded
}

TEST_F(MPMCTest, HighContention) {
    My::ProducerConsumerQueue<int> q(100);
    const int num_producers = 8;
    const int num_consumers = 8;
    const int items_per_thread = 10000;

    std::vector<std::thread> pool;
    std::atomic<int> total_popped{0};

    // Consumers
    for(int i=0; i<num_consumers; ++i) {
        pool.emplace_back([&]() {
            for(int j=0; j<items_per_thread; ++j) {
                int val = q.pop();
                total_popped++;
            }
        });
    }

    // Producers
    for(int i=0; i<num_producers; ++i) {
        pool.emplace_back([&]() {
            for(int j=0; j<items_per_thread; ++j) {
                q.push(1);
            }
        });
    }

    for(auto& t : pool) t.join();

    EXPECT_EQ(total_popped, num_producers * items_per_thread);
}
