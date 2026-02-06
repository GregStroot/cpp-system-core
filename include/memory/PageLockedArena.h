#pragma once
#include <cstddef>
#include <cstdint>
#include <sys/mman.h> // You will need this
#include <stdexcept>

namespace My {

    class PageLockedArena {
    public:
        // TODO:
        // 1. Use mmap to acquire 'size' bytes of memory.
        // 2. Use mlock to prevent this memory from being swapped to disk.
        // 3. Pre-fault the memory (write to every page) to force physical allocation now.
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
            for (int i=0; i < size_; i += page_size) {
                //TODO: Understand this! Why volatile
            }


        };

        // TODO: Clean up resources (munmap).
        ~PageLockedArena();

        // Prevent copying/moving (this is a resource handle)
        PageLockedArena(const PageLockedArena&) = delete;
        PageLockedArena& operator=(const PageLockedArena&) = delete;

        // TODO: Implement a simple bump allocator.
        // Return a pointer to the current offset and increment the offset.
        // Throw std::bad_alloc or return nullptr if out of memory.
        // (Optional: Handle alignment, but simple byte-granularity is acceptable for V1)
        void* allocate(size_t bytes);

        // Helper for the test suite to inspect internal state
        size_t capacity() const { return size_; }
        size_t used() const { return offset_; }

    private:
        void* memory_ = nullptr;
        size_t size_ = 0;
        size_t offset_ = 0;
        //Default page size is 4KB
        static constexpr page_size = 4096;
    };
}

