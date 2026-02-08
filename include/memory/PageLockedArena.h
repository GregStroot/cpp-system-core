#pragma once
#include <cstddef>
#include <cstdint>
#include <sys/mman.h> // You will need this
#include <stdexcept>

namespace My {

    class PageLockedArena {
    public:
        explicit PageLockedArena(size_t size): size_(size) {
            //Allocate
            void* ptr = mmap(nullptr, size_,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS,
                            -1, 0);

            if (ptr == MAP_FAILED) {
                throw std::runtime_error("mmap failed to allocate memory");
            }

            //Prevent pages from being stolen and removed from RAM
            int lockFlag = mlock(ptr, size_);
            //We failed to lock! Clean-up and those exception
            if (lockFlag != 0) {
                munmap(ptr, size_);
                throw std::runtime_error("mlock failed");
            }

            //Pre-fault the memory to pay the price up-front
            for (size_t i=0; i < size_; i += page_size) {
                //Ptr arithmetic is indexed by sizeof(T), so we use char
                //  that is one byte
                volatile char* p = static_cast<char*>(ptr) + i;
                *p = 0;
            }

            //Save it
            memory_ = ptr;
            offset_ = 0;
        };

        ~PageLockedArena() {
            //Unmap memory
            munmap(memory_, size_);
            //Everything else is dealt with already
        };

        // Prevent copying/moving (this is a resource handle)
        PageLockedArena(const PageLockedArena&) = delete;
        PageLockedArena& operator=(const PageLockedArena&) = delete;

        void* allocate(size_t bytes) {

            size_t aligned_offset = (offset_ + 7) & ~7;
            if (size_ - aligned_offset < bytes) {
                throw std::bad_alloc();
            }

            char* ptr = static_cast<char*>(memory_) + aligned_offset;

            offset_ = aligned_offset + bytes;

            return static_cast<void*>(ptr);
        };

        // Helper for the test suite to inspect internal state
        size_t capacity() const { return size_; }
        size_t used() const { return offset_; }

    private:
        void* memory_ = nullptr;
        size_t size_ = 0;
        size_t offset_ = 0;
        //Default page size is 4KB
        static constexpr size_t page_size = 4096;
        static constexpr size_t alignment = 8;
    };
}

