#pragma once
#include <atomic>
//Allows use of __mm_pause() on x86_64 and __i386__
#include <immintrin.h>

namespace My {

    // A user-space Spinlock using std::atomic_flag.
    // Must satisfy the BasicLockable concept to work with std::lock_guard.
    //
    // Usage:
    //   SpinLock lock;
    //   {
    //       std::lock_guard<SpinLock> guard(lock);
    //       // Critical section
    //   }
    class SpinLock {
    public:
        SpinLock() = default;

        // Spinlocks are strictly tied to the hardware address they reside at.
        // Copying or Moving them is nonsensical and dangerous.
        SpinLock(const SpinLock&) = delete;
        SpinLock& operator=(const SpinLock&) = delete;
        SpinLock(SpinLock&&) = delete;
        SpinLock& operator=(SpinLock&&) = delete;

        // Blocks the thread until the lock is acquired.
        // HFT Requirement: Must handle the "Busy Wait" efficiently (CPU Pipeline).
        void lock() {

            while (true) {
                //Try
                if (!flag_.test_and_set(std::memory_order_acquire)) {
                    return;
                }

                //Wait by looping read only
                // Prevent the expensive cache contention of test_and_set looping
                //      Which is a RMW operation
                while (flag_.test(std::memory_order_relaxed)) {
                    //std::thread::yield() is too slow 1000-1500ns
                    //Putting nothing here will fill the reorder buffer and requires a flush
                    //Using a CPU instruction allows us to actually sleep for 50ns
                    //  without implicating the OS (like std::chrono would)
                    _mm_pause();

                    //Upon testing __mm_pause() is actually slower than burning CPU
                    //  at least for a small two thread test with 10e6 iterations
                }
            }
        };

        // Attempts to acquire the lock without blocking.
        // Returns true if successful, false if the lock was already taken.
        bool try_lock() {
            return !flag_.test_and_set(std::memory_order_acquire);
        };

        // Releases the lock.
        // Requirement: Must verify what memory ordering is required here.
        void unlock() {
            flag_.clear(std::memory_order_release);
        };

    private:
        // The only member variable you need.
        // Guaranteed to be lock-free on almost all modern hardware.
        std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

    };
}

