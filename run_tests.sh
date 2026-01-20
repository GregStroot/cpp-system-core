#!/bin/bash
set -e  # Stop immediately if any command fails

# 1. Ensure build directory exists
if [ ! -d "build" ]; then
    echo "📂 Creating build directory..."
    mkdir build
fi

# 2. Configure (Generate Makefiles)
# We hide output unless there is an error to keep the terminal clean
echo "⚙️  Configuring CMake..."
cmake -S . -B build > /dev/null

# 3. Build (Compiles modified files)
echo "🚀 Building..."
cmake --build build --parallel 4  # Use 4 cores for speed

# 4. Test
echo "🧪 Running Tests..."
cd build

# Logic:
# If no arguments -> Run ALL tests
# If argument provided -> Run regex match (e.g., ./run_tests.sh Array)
if [ -z "$1" ]; then
    ctest --output-on-failure
else
    echo "🔎 Filtering for: $1"
    ctest -R "$1" --output-on-failure
fi
