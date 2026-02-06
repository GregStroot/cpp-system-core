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
- [x] **`Vector<T>`**:
- [x] **`Array<T, N>`**:
- [x] **`SharedPtr<T>`**:
    - [x] **`make_shared`**
- [x] **`UniquePtr<T>`**:
    - [x] **`make_unique`**
    - [ ] Custom deleter constructor
- [ ] **`PageLockedArena`**:
- [ ] **`ObjectPool`**:

### 2. Lock-Free & Queues
- [x] **`SpscRing<T>`**:
    - [x] `seq_cst` version
    - [x] `acquire/release` version
- [x] **`CircularBuffer<T>`**:
- [ ] **`ConflationQueue<T>`**:
- [ ] **`ProducerConsumer<T>`**:

### 3. Concurrency Primitives
- [x] **`Mutex`**:
- [x] **`Spinlock`**:
- [x] **`Semaphore`**:
- [x] **`RWLock`**:
    - Lock free via atomic's
- [ ] **`ThreadPool`**:
- [ ] **`Seqlock`**:
    - Optional, easy to write; conceptually interesting

### 4. Systems Components
- [ ] **`OrderBook`**:
- [ ] **`LRUCache<K, V>`**:
- [ ] **`RateLimiter`**:
- [ ] **`AsyncLogger`**:
    - Optional, useful in production

### 5. Market
- [ ] **`ITCHHandler`**:

## Build & Test
Dependencies: CMake 3.14+, GoogleTest (fetched automatically).

```bash
cmake -S . -B build
cmake --build build
cd build && ctest --output-on-failure
