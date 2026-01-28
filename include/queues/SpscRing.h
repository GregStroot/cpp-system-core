#pragma once
#include <atomic>
#include <cstddef>
#include <memory>

/*
   Design thoughts:

   * We have two threads:
        1. Writes to head, reads tail
        2. Writes to tail, reads head

    * Design challenge:
        - We need to ensure that resources are synced before push/pop
        - If we aren't careful, it's possible that head/tail are incremented but
        we don't have the actual buffer updated

    * We can fix this in seq_cst, but ensure each access or the head/tail is atomic
        - As well as ensuring the tail is incremented atomically
        - This is overly restrictive, but we start here

*/

namespace My {

    template<typename T>
    class SpscRing {
    public:
        // Allocates the buffer of given size.
        explicit SpscRing(size_t size):
            capacity_(size+1),
            size_(size),
            head_(0),
            tail_(0)
        {
            //We need a sacraficial slot to determine empty/full
            buff_.resize(capacity_);
        };

        ~SpscRing() = default;

        // Non-copyable, Non-movable
        SpscRing(const SpscRing&) = delete;
        SpscRing& operator=(const SpscRing&) = delete;
        SpscRing(SpscRing&&) = delete;
        SpscRing& operator=(SpscRing&&) = delete;

        // --- Core Operations ---

        // Returns true if successful, false if full.
        bool push(const T& item) {
            const size_t curr_tail = tail_.load(std::memory_order_seq_cst);
            const size_t next_tail = (curr_tail + 1) % capacity_;
            //Producer therefore no one modifies tail_
            if ( next_tail != head_.load()) {
                //If head not directly in front of tail, we can push
                buff_[tail_] = item;
                tail_.store(next_tail);
                return true;
            }
            else {
                return false;
            }
        };
        bool push(T&& item) {;
            const size_t curr_tail = tail_.load(std::memory_order_seq_cst);
            const size_t next_tail = (curr_tail + 1) % capacity_;
            if ( next_tail != head_.load()) {
                //If head not directly in front of tail, we can push
                buff_[curr_tail] = std::move(item);
                tail_.store(next_tail);
                return true;
            }
            else {
                return false;
            }
        };

        // Returns true if successful, false if empty.
        bool pop(T& output) {
            if (head_.load() != tail_.load()) {
                output = std::move(buff_[head_]);
                head_.store((head_ + 1) % capacity_);
                return true;
            }
            else {
                return false;
            }

        };

        // --- Observers ---

        bool empty() const noexcept {
            return head_ == tail_;
        };
        bool full() const noexcept {
            return ((tail_ + 1) % capacity_) == head_;
        };
        size_t size() const noexcept {
            const size_t curr_tail = tail_.load();
            const size_t curr_head = head_.load();

            if (curr_head > curr_tail) {
                return (capacity_ - curr_head + 1) + curr_tail;
            }
            else {
                return curr_tail - curr_head;
            }
        };
        size_t capacity() const noexcept {
            return size_;
        };

    private:
        const size_t capacity_;
        const size_t size_;
        std::vector<T> buff_;
        //To prevent cache contention, you MUST align them
        alignas(64) std::atomic<int> head_;
        alignas(64) std::atomic<int> tail_;
    };
}

