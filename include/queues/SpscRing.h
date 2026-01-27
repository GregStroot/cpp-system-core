#pragma once
#include <atomic>
#include <cstddef>
#include <memory>

namespace My {

    template<typename T>
    class SpscRing {
    public:
        // Allocates the buffer of given size.
        explicit SpscRing(size_t size);

        ~SpscRing();

        // Non-copyable, Non-movable
        SpscRing(const SpscRing&) = delete;
        SpscRing& operator=(const SpscRing&) = delete;
        SpscRing(SpscRing&&) = delete;
        SpscRing& operator=(SpscRing&&) = delete;

        // --- Core Operations ---

        // Returns true if successful, false if full.
        bool push(const T& item);
        bool push(T&& item);

        // Returns true if successful, false if empty.
        bool pop(T& output);

        // --- Observers ---

        bool empty() const noexcept;
        bool full() const noexcept;
        size_t size() const noexcept;
        size_t capacity() const noexcept;

    private:
        // You determine the necessary members and layout.
        // Remember the constraints discussed in Day 2.
    };
}

