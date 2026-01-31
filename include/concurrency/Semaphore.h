#pragma once
#include <mutex>
#include <condition_variable>
#include <cstddef>

namespace My {

    class Semaphore {
    public:
        // Initialize the semaphore with a specific count.
        // count: The number of threads allowed to access the resource simultaneously.
        explicit Semaphore(size_t initial_count = 0):
            count_(initial_count)
        {};

        ~Semaphore() = default;

        // Non-copyable, Non-movable
        Semaphore(const Semaphore&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;
        Semaphore(Semaphore&&) = delete;
        Semaphore& operator=(Semaphore&&) = delete;

        // --- Core Operations ---

        // Increments the internal counter and unblocks a waiting thread (if any).
        // Often called "post" or "signal" or "V".
        void release() {
            //Update counter
            {
                std::unique_lock<std::mutex> lg(mutex_);
                count_++;
            }
            cv_.notify_one();
            //Send a signal
        };

        // Decrements the internal counter.
        // If the counter is 0, blocks the calling thread until the counter > 0.
        // Often called "wait" or "P".
        void acquire() {
            std::unique_lock<std::mutex> lg(mutex_);
            cv_.wait(lg, [&](){return count_ > 0;} );
            count_--;
        };

        // Tries to decrement the internal counter without blocking.
        // Returns true if successful (counter was > 0).
        // Returns false immediately if counter was 0.
        bool try_acquire() {
            std::unique_lock<std::mutex> lg(mutex_);
            if (count_ == 0) {
                return false;
            }
            else  {
                count_--;
                return true;
            }

        };

    private:
        std::mutex mutex_;
        std::condition_variable cv_;
        size_t count_;
    };
}

