# Memory Tests
add_executable(memory_tests memory_tests.cpp)
target_link_libraries(memory_tests GTest::gtest_main)
# Add Sanitizers for memory debugging (Optional but recommended)
target_compile_options(memory_tests PRIVATE -fsanitize=address -g)
target_link_options(memory_tests PRIVATE -fsanitize=address)
add_test(NAME MemoryTests COMMAND memory_tests)

# Concurrency Tests (Needs Threads)
add_executable(concurrency_tests concurrency_tests.cpp)
target_link_libraries(concurrency_tests GTest::gtest_main pthread)
# Use Thread Sanitizer for these tests
target_compile_options(concurrency_tests PRIVATE -fsanitize=thread -g)
target_link_options(concurrency_tests PRIVATE -fsanitize=thread)
add_test(NAME ConcurrencyTests COMMAND concurrency_tests)

# Systems Tests
add_executable(systems_tests systems_tests.cpp)
target_link_libraries(systems_tests GTest::gtest_main)
add_test(NAME SystemsTests COMMAND systems_tests)
