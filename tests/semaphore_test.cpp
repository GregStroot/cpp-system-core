#include <gtest/gtest.h>
#include "concurrency/Semaphore.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

class SemaphoreTest : public ::testing::Test {};

// 1. Basic Single-Threaded Logic
TEST_F(SemaphoreTest, InitialCountBehavior) {
    // Start with 1 permit
    My::Semaphore sem(1);

    // Should succeed
    EXPECT_TRUE(sem.try_acquire());

    // Should fail (count is now 0)
    EXPECT_FALSE(sem.try_acquire());

    // Release 1
    sem.release();

    // Should succeed again
    EXPECT_TRUE(sem.try_acquire());
}

// 2. The "Ping Pong" (Strict Ordering)
// A common use of Semaphores is to force Thread A to run before Thread B.
TEST_F(SemaphoreTest, SignalingOrder) {
    My::Semaphore sem(0); // Start locked
    std::vector<int> execution_order;

    std::thread t([&]() {
        // Block until main thread releases
        sem.acquire();
        execution_order.push_back(2);
    });

    // Ensure thread has started and is blocked
    // (Sleep is crude, but effective for checking simple blocking behavior)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    execution_order.push_back(1);

    // Unblock the thread
    sem.release();
    t.join();

    ASSERT_EQ(execution_order.size(), 2);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], 2);
}

// 3. Mutex Simulation (Binary Semaphore)
TEST_F(SemaphoreTest, ActsAsMutex) {
    My::Semaphore sem(1); // Capacity 1 acts like a Mutex
    int shared_counter = 0;
    const int iterations = 10000;

    auto task = [&]() {
        for (int i = 0; i < iterations; ++i) {
            sem.acquire();
            shared_counter++;
            sem.release();
        }
    };

    std::thread t1(task);
    std::thread t2(task);

    t1.join();
    t2.join();

    EXPECT_EQ(shared_counter, 2 * iterations);
}

// 4. The "Permit" Test (Counting functionality)
TEST_F(SemaphoreTest, AllowsMultipleAccess) {
    const int capacity = 3;
    My::Semaphore sem(capacity);

    std::atomic<int> active_threads{0};
    std::atomic<bool> max_concurrency_reached{false};

    std::vector<std::thread> threads;
    for (int i = 0; i < 6; ++i) {
        threads.emplace_back([&]() {
            sem.acquire();

            // Critical Section
            int current = ++active_threads;
            if (current > capacity) {
                // If this ever happens, the semaphore failed
                // (Though race conditions usually make this flaky to detect,
                // atomic makes it strictly ordered in time)
                ADD_FAILURE() << "More threads entered than allowed!";
            }
            if (current == capacity) {
                max_concurrency_reached = true;
            }

            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            --active_threads;
            sem.release();
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(max_concurrency_reached);
}
