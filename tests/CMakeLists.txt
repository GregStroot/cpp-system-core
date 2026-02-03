include(GoogleTest) # Required for gtest_discover_tests

# --- Common Flags ---
# We use AddressSanitizer (ASan) to catch buffer overflows in Array/Vector.
# This is non-negotiable for Systems Programming.
set(MEMORY_FLAGS -fsanitize=address -g)

# ==========================================
# 1. Day 1 & 2: Memory (Array, Vector, Pointers)
# ==========================================
add_executable(memory_tests memory_tests.cpp)
target_link_libraries(memory_tests GTest::gtest_main)

# Apply Sanitizers
target_compile_options(memory_tests PRIVATE ${MEMORY_FLAGS})
target_link_options(memory_tests PRIVATE ${MEMORY_FLAGS})

# MAGIC LINE: Parses the C++ file to register every TEST() individually
gtest_discover_tests(memory_tests)

add_executable(vector_tests vector_tests.cpp)
target_link_libraries(vector_tests GTest::gtest_main)
gtest_discover_tests(vector_tests)

add_executable(unique_ptr_tests unique_ptr_tests.cpp)
target_link_libraries(unique_ptr_tests GTest::gtest_main)
gtest_discover_tests(unique_ptr_tests)

add_executable(shared_ptr_tests shared_ptr_tests.cpp)
target_link_libraries(shared_ptr_tests GTest::gtest_main)
gtest_discover_tests(shared_ptr_tests)

# ==========================================
# 2. Day 3-8: Concurrency (LockFree, Threads)
# ==========================================

add_executable(circular_buffer_tests circular_buffer_tests.cpp)
target_link_libraries(circular_buffer_tests GTest::gtest_main)
gtest_discover_tests(circular_buffer_tests)

add_executable(spsc_tests spsc_tests.cpp)
target_link_libraries(spsc_tests GTest::gtest_main)
gtest_discover_tests(spsc_tests)

add_executable(spinlock_tests spinlock_tests.cpp)
target_link_libraries(spinlock_tests GTest::gtest_main)
gtest_discover_tests(spinlock_tests)

add_executable(semaphore_test semaphore_test.cpp)
target_link_libraries(semaphore_test GTest::gtest_main)
gtest_discover_tests(semaphore_test)

add_executable(mutex_tests mutex_tests.cpp)
target_link_libraries(mutex_tests GTest::gtest_main)
gtest_discover_tests(mutex_tests)

#add_executable(mpmc_tests mpmc_tests.cpp)
#target_link_libraries(mpmc_tests GTest::gtest_main)
#gtest_discover_tests(mpmc_tests)

add_executable(rwlock_tests rwlock_tests.cpp)
target_link_libraries(rwlock_tests GTest::gtest_main)
gtest_discover_tests(rwlock_tests)

# ==========================================
# 3. Day 9-10: Systems (OrderBook)
# ==========================================
# Uncomment when you reach Day 9
# add_executable(systems_tests systems_tests.cpp)
# target_link_libraries(systems_tests GTest::gtest_main)
# gtest_discover_tests(systems_tests)
