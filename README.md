# C++ Systems Core Data structure implementations

A from-scratch implementation of core data structures using C++ 20.

**Goal:** To understand the `mechanical sympathy' required for low-latency engineering by building primitives used in STL

**Constraints:**
* No external dependencies (other than GoogleTest)
* No raw `new` or `delete` (RAII only)
* Zero-copy and cache-friendly design focus


## Artifacts Implemented(/To be implemented)

We are breaking them down into four categories:

### 1. Memory & Ownership
- [ ] **`Vector<T>`**:
- [ ] **`Array<T, N>`**:
- [ ] **`SharedPtr<T>`**:
- [ ] **`UniquePtr<T>`**:

### 2. Lock-Free & Queues
- [ ] **`SpscRing<T>`**:
- [ ] **`CircularBuffer<T>`**:
- [ ] **`ConflationQueue<T>`**:
- [ ] **`ProducerConsumer<T>`**:

### 3. Concurrency Primitives
- [ ] **`Spinlock`**:
- [ ] **`Semaphore`**:
- [ ] **`RWLock`**:
- [ ] **`ThreadPool`**:

### 4. Systems Components
- [ ] **`OrderBook`**:
- [ ] **`LRUCache<K, V>`**:

## Build & Test
Dependencies: CMake 3.14+, GoogleTest (fetched automatically).

```bash
cmake -S . -B build
cmake --build build
cd build && ctest --output-on-failure
