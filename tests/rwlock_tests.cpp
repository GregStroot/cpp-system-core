#include <gtest/gtest.h>
#include "concurrency/RWLock.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

class RWLockTest : public ::testing::Test {};

// 1. Basic Readers (Shared)
TEST_F(RWLockTest, MultipleReaders) {
    My::RWLock rw;
    std::atomic<int> read_count{0};
    const int num_readers = 10;

    // Acquire read lock, increment atomic, hold for a bit
    auto reader_task = [&]() {
        rw.acquire_read();
        read_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        rw.release_read();
    };

    std::vector<std::thread> threads;
    for(int i=0; i<num_readers; ++i) threads.emplace_back(reader_task);

    // Give them time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // They should ALL be in. If this was a mutex, count would be 1.
    // Note: This relies on the OS scheduling them quickly, but 50ms is usually enough.
    EXPECT_GT(read_count.load(), 1);

    for(auto& t : threads) t.join();
}

// 2. Writer Exclusivity
TEST_F(RWLockTest, WriterExclusivity) {
    My::RWLock rw;
    int shared_data = 0;

    auto writer = [&]() {
        rw.acquire_write();
        shared_data++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        rw.release_write();
    };

    auto reader = [&]() {
        rw.acquire_read();
        // If reader enters while writer is active, it might see partial data (race)
        // TSAN will catch this if locking is broken.
        int val = shared_data;
        rw.release_read();
    };

    std::thread t1(writer);
    std::thread t2(reader);
    std::thread t3(reader);

    t1.join(); t2.join(); t3.join();
}

// 3. THE CRITICAL TEST: Writer Priority
TEST_F(RWLockTest, WriterPriority) {
    My::RWLock rw;

    // Step 1: Block the lock with a "Long Reader"
    rw.acquire_read();

    std::atomic<bool> writer_entered{false};
    std::atomic<bool> short_reader_entered{false};

    // Step 2: Launch a Writer (Should block, but register intent)
    std::thread t_writer([&]() {
        rw.acquire_write();
        writer_entered = true;
        rw.release_write();
    });

    // Short sleep to ensure Writer hits the lock and waits
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Step 3: Launch a NEW Reader (Should be BLOCKED by the waiting writer)
    std::thread t_reader([&]() {
        rw.acquire_read();
        short_reader_entered = true;
        rw.release_read();
    });

    // Step 4: Verify state BEFORE releasing the Long Reader
    // Writer is waiting (correct)
    EXPECT_FALSE(writer_entered);
    // New Reader should ALSO be waiting (Writer Priority in effect!)
    EXPECT_FALSE(short_reader_entered);

    // Step 5: Release Long Reader
    rw.release_read();

    t_writer.join();
    t_reader.join();

    EXPECT_TRUE(writer_entered);
    EXPECT_TRUE(short_reader_entered);
}
