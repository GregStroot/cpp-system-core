#include <gtest/gtest.h>
#include "concurrency/SpinLock.h"
#include <thread>
#include <vector>
#include <mutex> // For std::lock_guard

class SpinLockTest : public ::testing::Test {};

// 1. Basic Mechanics (Single Thread)
TEST_F(SpinLockTest, BasicLockUnlock) {
    My::SpinLock spinlock;

    // Should be able to lock
    spinlock.lock();

    // Critical section (simulated)
    int critical_resource = 1;
    EXPECT_EQ(critical_resource, 1);

    // Should be able to unlock
    spinlock.unlock();
}

// 2. TryLock Logic
TEST_F(SpinLockTest, TryLock) {
    My::SpinLock spinlock;

    // First try should succeed
    EXPECT_TRUE(spinlock.try_lock());

    // Second try (on same thread) should fail because we hold it
    // Note: SpinLocks are usually NOT recursive.
    EXPECT_FALSE(spinlock.try_lock());

    spinlock.unlock();

    // Should be available again
    EXPECT_TRUE(spinlock.try_lock());
    spinlock.unlock();
}

// 3. Mutual Exclusion (Two Threads)
TEST_F(SpinLockTest, MutualExclusion) {
    My::SpinLock spinlock;
    int counter = 0;

    // Lock it in main thread
    spinlock.lock();

    // Spawn a thread that tries to acquire it
    std::thread t([&]() {
        if (spinlock.try_lock()) {
            counter++; // Should NOT happen
            spinlock.unlock();
        }
    });

    t.join();

    // Counter should still be 0 because thread couldn't get in
    EXPECT_EQ(counter, 0);

    spinlock.unlock();
}

// 4. The HFT Stress Test (High Contention)
// If your lock is broken, 'counter' will be less than expected due to race conditions.
TEST_F(SpinLockTest, StressTest) {
    My::SpinLock spinlock;
    int counter = 0;
    const int num_threads = 8; // Enough to saturate most dev machines
    const int increments_per_thread = 100'000;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < increments_per_thread; ++j) {
                // Manually lock/unlock to test raw primitive
                spinlock.lock();
                counter++;
                spinlock.unlock();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Mathematical Proof of Correctness
    EXPECT_EQ(counter, num_threads * increments_per_thread);
}

// 5. Standard Library Integration
// Verifies your signature matches the "BasicLockable" concept
TEST_F(SpinLockTest, WorksWithLockGuard) {
    My::SpinLock spinlock;
    int counter = 0;

    {
        std::lock_guard<My::SpinLock> guard(spinlock);
        counter = 1;
    } // Should unlock here

    // If unlock failed or crashed, this next lock would hang/fail
    spinlock.lock();
    EXPECT_EQ(counter, 1);
    spinlock.unlock();
}
