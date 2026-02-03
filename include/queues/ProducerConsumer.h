#pragma once
#include <mutex>
#include <condition_variable>
#include "queues/CircularBuffer.h" // Your day 3 artifact

namespace My {

    template<typename T>
    class ProducerConsumerQueue {
    public:
        explicit ProducerConsumerQueue(size_t capacity) : buffer_(capacity) {}

        // Blocks if full.
        void push(const T& item);

        // Blocks if empty.
        T pop();

        // Non-blocking attempts (Optional, but good for HFT)
        bool try_push(const T& item);
        bool try_pop(T& item);

    private:

    };
}

