#pragma once
#include <cstddef>
#include <memory>
#include <vector>

namespace My {

    template<typename T>
    class CircularBuffer {
    public:
        // Allocates the buffer of given size.
        explicit CircularBuffer(size_t capacity):
            capacity_(capacity),
            currSize_(0),
            head_(0),
            tail_(0)
        {
            buff.resize(capacity_);
        };

        ~CircularBuffer() = default;

        // --- Core Operations ---

        // Adds item to the buffer.
        // Returns true if successful.
        // Returns false if buffer is full.
        bool push(const T& item) {
            if (currSize_ == capacity_) {
                return false;
            }
            else {
                //TODO: Have we done the ordering properly?
                buff[tail_] = item;
                tail_= (tail_+1) % capacity_;
                currSize_++;
                return true;
            }
        };
        bool push(T&& item) {
            if (currSize_ == capacity_) {
                return false;
            }
            else {
                //TODO: Have we done the ordering properly?
                //rvalue should be automatically handled by the vector place
                buff[tail_] = std::forward<T>(item);
                tail_= (tail_+1) % capacity_;
                currSize_++;
                return true;
            }
        };

        // Removes item from the buffer.
        // Returns true if successful (writes to output).
        // Returns false if empty.
        bool pop(T& output) {
            if (currSize_ == 0) {
                return false;
            }
            else {

                //We don't need it anymore
                output = std::move(buff[head_]);
                head_ = (head_ +1) % capacity_;
                currSize_--;
                return true;
            }
        };

        // --- Observers ---

        bool empty() const noexcept {
            return currSize_ == 0;
        };
        bool full() const noexcept {
            return currSize_ == capacity_;
        };
        size_t size() const noexcept {
            return currSize_;
        };
        size_t capacity() const noexcept {
            return capacity_;
        };

    private:
        const size_t capacity_;
        std::vector<T> buff;
        size_t currSize_;
        size_t head_;
        size_t tail_;
        // Implementation details
    };
}

