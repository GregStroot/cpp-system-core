#pragma once
#include <atomic>
#include <cstdint>
//Allows use of __mm_pause() on x86_64 and __i386__
#include <immintrin.h>

static_assert(std::atomic<uint32_t>::is_always_lock_free, "This platform is too weak for HFT!");

namespace My {

    class Mutex {
    public:
        Mutex() = default;

        // Non-copyable/movable
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;

        // Blocks until lock is acquired.
        // 1. Spin for 'kSpinCount' iterations.
        //      -- Optimisation for short wait times
        // 2. If still locked, sleep (using std::atomic::wait or syscall).
        //      -- E.g. OS lock (slow)
        void lock() {

            while (true) {
                uint64_t ready    = 0;
                //Only unlock if we have 0 already
                if (flag_.compare_exchange_strong(ready,1, std::memory_order_acquire)) {
                    return;
                }

                //Pause for specific time
                for (int i=0; i < kSpinCount; i++) {
                    _mm_pause();
                }

                //Try again
                //Test-and-set
                ready    = 0;
                if (flag_.compare_exchange_strong(ready,1, std::memory_order_acquire)) {
                    return;
                }
                else {
                    uint64_t waiting  = 1;
                    flag_.compare_exchange_strong(waiting,2, std::memory_order_acquire);
                    flag_.wait(2, std::memory_order_relaxed);
                }

            }

        };

        // Tries to acquire lock without blocking.
        // Returns true if successful.
        bool try_lock() {
            uint64_t ready    = 0;
            return flag_.compare_exchange_strong(ready, 1, std::memory_order_acquire);
        };

        // Releases the lock and wakes up *one* waiter (if any).
        void unlock() {
            //Optimisation to not require notify if there are no waiters
            if (flag_.exchange(0, std::memory_order_release) == 2) {
                flag_.notify_one();
            }
        };

    private:
        //0 atomic is free
        //1 atomic is locked (hopefully no waiters!)
        //2 atomic is locked + waiters
        std::atomic<uint64_t> flag_ = 0;

        static constexpr uint64_t kSpinCount = 10;
    };
}

