#include <gtest/gtest.h>
#include "memory/PageLockedArena.h"
#include <cstring>
#include <vector>

class ArenaTest : public ::testing::Test {};

// 1. Can we construct it without crashing? (Checks mmap/mlock return values implicitly)
TEST_F(ArenaTest, Construction) {
    EXPECT_NO_THROW({
        My::PageLockedArena arena(1024 * 1024); // 1MB
    });
}

// 2. Does allocate return distinct, advancing pointers?
TEST_F(ArenaTest, SimpleBumpAllocation) {
    size_t size = 1024;
    My::PageLockedArena arena(size);

    void* p1 = arena.allocate(100);
    void* p2 = arena.allocate(100);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p1, p2);

    // Cast to uintptr_t to check address math
    auto addr1 = reinterpret_cast<uintptr_t>(p1);
    auto addr2 = reinterpret_cast<uintptr_t>(p2);

    // p2 must be at least 100 bytes ahead of addr1 (due to padding),
    //      addr1 to next address multiple of alignment
    EXPECT_GE(addr2 - addr1, 100);
    EXPECT_EQ((addr2 - addr1) % 8, 0);
}

// 3. Can we actually write to the memory? (Segfault check)
TEST_F(ArenaTest, WriteCheck) {
    My::PageLockedArena arena(4096);
    int* numbers = static_cast<int*>(arena.allocate(sizeof(int) * 100));

    for(int i = 0; i < 100; ++i) {
        numbers[i] = i; // This should not segfault
    }

    EXPECT_EQ(numbers[50], 50);
}

// 4. Does it correctly detect OOM?
TEST_F(ArenaTest, OutOfMemoryProtection) {
    My::PageLockedArena arena(100); // Tiny arena
    arena.allocate(80); // Success

    // Attempt to allocate more than remains
    EXPECT_THROW({
        void* ptr = arena.allocate(21);
        if (ptr == nullptr) throw std::bad_alloc(); // Map nullptr to throw for test consistency
    }, std::bad_alloc);
}

// 5. Large Allocation / Stress Test
TEST_F(ArenaTest, LargeAllocation) {
    // 100MB Arena
    size_t large_size = 100 * 1024 * 1024;
    My::PageLockedArena arena(large_size);

    // Allocate 10MB chunks
    for(int i = 0; i < 9; ++i) {
        void* ptr = arena.allocate(10 * 1024 * 1024);
        ASSERT_NE(ptr, nullptr);
        // Write to the start and end of the chunk to verify accessibility
        char* bytes = static_cast<char*>(ptr);
        bytes[0] = 0xFF;
        bytes[(10 * 1024 * 1024) - 1] = 0xFF;
    }

    EXPECT_EQ(arena.used(), 90 * 1024 * 1024);
}
