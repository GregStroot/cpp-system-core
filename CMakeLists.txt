cmake_minimum_required(VERSION 3.14)
project(sig_prep CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clangd/VSCode intellisense

# Enable testing
enable_testing()

# 1. Fetch GoogleTest automatically
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# Prevent overriding parent project compiler settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 2. Include your header files
include_directories(include)

# 3. Add subdirectories
add_subdirectory(tests)
