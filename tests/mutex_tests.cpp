#include <gtest/gtest.h>
#include "concurrency/Mutex.h"
#include <thread>
#include <vector>
#include <mutex> // For std::lock_guard

class MutexTest : public ::testing::Test {};

// 1. Basic Locking
TEST_F(MutexTest, BasicLockUnlock) {
    My::Mutex mtx;
    int protected_data = 0;

    mtx.lock();
    protected_data = 1;
    mtx.unlock();

    EXPECT_EQ(protected_data, 1);
}

// 2. TryLock Semantics
TEST_F(MutexTest, TryLock) {
    My::Mutex mtx;

    // First take succeeds
    EXPECT_TRUE(mtx.try_lock());

    // Second take fails (recursive locking is NOT supported)
    EXPECT_FALSE(mtx.try_lock());

    mtx.unlock();

    // Succeeds again
    EXPECT_TRUE(mtx.try_lock());
    mtx.unlock();
}

// 3. Mutual Exclusion (The Core Test)
TEST_F(MutexTest, Exclusion) {
    My::Mutex mtx;
    int counter = 0;
    const int iterations = 10000;

    auto worker = [&]() {
        for (int i = 0; i < iterations; ++i) {
            mtx.lock();
            counter++;
            mtx.unlock();
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);

    t1.join();
    t2.join();

    // If exclusion failed, counter would be < 20000 due to race conditions
    EXPECT_EQ(counter, 2 * iterations);
}

// 4. Heavy Contention (Force the Hybrid Sleep)
// This test spawns enough threads to force the mutex into "Sleep Mode"
// (assuming kSpinCount isn't infinite).
TEST_F(MutexTest, HighContention) {
    My::Mutex mtx;
    int counter = 0;
    const int num_threads = 16;
    const int increments = 1000;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < increments; ++j) {
                std::lock_guard<My::Mutex> lock(mtx);
                counter++;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, num_threads * increments);
}
