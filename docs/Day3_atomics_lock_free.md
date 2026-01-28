
# Chapter 5: The C++ memory model and operations on atomic types

**One of the goals of the Standards Committee is that there will be no need for a lower-level language than C++**

## 5.1: Memory model basics

* There are two aspects to the memory model:
    1. The basic *structural* aspects
    2. The *concurrency* aspects

### 5.1.1: Objects and memory locations

* All data in a C++ program is made up of **objects**
    - The Standard defines an object as "a region of storage," although it goes on to assign properties to these objects, such as type and lifetime

* There are four important points to take away from this:
    * Every variable is an object, including those that are members of other objects
    * Every object occupies *at least one* memory location
    * Variables of fundamental types, such as `int` or `char` occupy *exactly one* memory location, whatever their size is, even if they're adjacent or part of an array
    * Adjacent bit fields are part of the same memory location!
        * Meaning they are not thread-safe!

### 5.1.2: Objects, memory locations, and concurrency

* If two threads access the *same* memory location, (if writing) there must be an **enforced ordering** between the access in the two threads
    - Can be done with mutexs or atomic operations
    - If there is not an enforced ordering, we have a data race and it causes **undefined behavior**
    - **Undefined behavior is one of the nastiest corners of C++**
    * Although using atomic conditions does *not* prevent the data race, it does bring the program back to the realm of defined behavior

### 5.1.3: Modification orders

* Every object in C++ has a *modication order* composed of all the write to that object from all threads in the program
    - Starting with the objects initialisation
    - In most cases, the order varies per run, but in any given execution of the program all threads in the system must agree on the order

* If the object in question isn't one of the atomic types, you're responsible for making certain that there's sufficient synchronization to ensure that threads agree on modification order of each variable

## 5.2: Atomic operations and types in C++

* An *atomic operation* is an indivisible operation
    - You can't observe the operation in a half-done state from any thread -- it's either done or not done

* Non-atomic operations might be seen as half-done!
    - If non-atomic operation is composed of atomic operations (e.g. assignment to a `struct` with `atomic` members), then other threads may only observe a subset of the atomic operations

### 5.2.1: The standard atomic types

* The key use case for atomic operations is as a replacement for an operation that would otherwise use a mutex for synchronization
    - Sometimes the internal atomic operation may use a mutex, resulting in no performance gains!
        * This is an important use case for lock-free data structures
        * This is so important that the library provides macros to determine at compile time whether the atomic types are lock-free

* The only atomic object that doesn't have `X::is_lock_free()` is `std::atomic_flag` which is *required* to be lock-free
    - Hence it can be used to implement other atomics! (or spinlock)

* Atomics are not copyable or assignable in the conventional sense
     - they have no copy or copy assignment constructors

* **NOTE**: Skipped a lot of specifics regarding functionality of `std::atomic<>`

### 5.2.2: Operations on `std::atomic_flag`

* `atomic_flag` can be in one of two states:
    * set
    * clear

* **SKIPPED OVER**: We need more context to appreciate this!
    - We will come back during spinlock implementation
*

## 5.3 Synchronizing operations and enforcing ordering

### 5.3.1 The synchronizes-with relationship

* Can only get between operations on atomic types

* *Sychronize-with* means what you might expect:
    * If thread A stores a value and thread B reads that value, there's a synchronize-with relationship between the store and the load.
    * This means the atomic flag being modifies (or atomic operation guarantees that) that all non-atomic data you wrote before the flag (**the payload**) has safely crossed the bridge and it available to the reader

* Synchronize-with does require "suitably-tagged" operations and the C++ memory model allows various ordering constraints
    - Covered in 5.3.3

### 5.3.2 The happens-before relationship

* *Happens-before* and *Strongly-happens-before* relationships are the basic building blocks of operation ordering in a program
    - It specifies which operations see the effect of the other operations

* For a single thread, it's straightforward:
    * If one op. is sequenced before another, then it also happens before it, and strongly-happens before it
        - Note the **"As-If rule"** (the 'get out of jail free' card from the C++ standard):
            - The compiler can perform any transformation it wants **AS LONG AS** the observed behavior of the single-threaded program remains identical to what the source code dictates

* Happens-before relations are the only things stopping multi-threaded programs from falling completely apart


### 5.3.3 Memory ordering for atomic operations


* There are six memory ordering options that can be applied to operations on atomic types:
    * `memory_order_relaxed`
    * `memory_order_consume`
    * `memory_order_acquire`
    * `memory_order_release`
    * `memory_order_acq_release`
    * `memory_order_seq_cst`

* By default, the memory-ordering option is: `memory_order_seq_cst`
    - Which is the most stringent

* Although there are six ordering options, they represent three models:
    * *Sequentially consistent ordering*
        * `memory_order_seq_cst`
    * *Acquire-release* ordering
        * `memory_order_consume`, `memory_order_acquire`, `memory_order_release`, `memory_order_acq_rel`
    * *Relaxed* ordering
        * `memory_order_relaxed`


#### Sequentially consistent ordering

* Sequentially consistent implies that the behavior is consistent with a simply sequential view of the world

* The ease of understanding comes at a price!
    * On a weakly-ordered machine with many processors, it can impose a noticeable performance penalty, because the overall sequence of operations must be kept consistent between the processors -- requiring extensive and expensive synchronization operations
        - That being said some architectures (such as the common x86 and x86-64) offer sequential consistency for relatively cheap
            * Though 'Cheap' is is still 50-100 cycles! (Uses `LOCK XCHG` instead of `MOV`)
            *
#### Non-sequentially consistent ordering

* Once we step out of `seq_cst` there is **no longer a single global order of event**!
    * Threads do not even need to agree on the order of events!

* In the absence of any other ordering constraints, the only requirement is that all threads agree on the modification order of each individual variable

#### Relaxed ordering

* Operations on atomic types performed with relaxed ordering do not participate in sychronizes-with relationships!
    - E.g. we don't guarantee that the payload before the atomic type with relaxed has arrived before we send the signal!

* Operations on the same variable within a single thread still obey happens-before relationships
    - But there is no requirement relative to other threads

#### Understanding Relaxed Ordering

* **EXCELLENT EXPLAINATION** with a man in a cubicle with a notepad

* We have no guarantee of ordering relative to different threads -- just that no time-travel occurs on a specific thread
    - This is because there is no absolute universal time
    - It's like a light-tower and something may see red even when another thread turned it to green
        * Because light takes time to travel (just as data stores/sends do!)!
    - Physically what happens is that the store buffer on a CPU may be changed, but the L1 cache not invalidated yet, so another thread could hop in between those two moments


* I strongly recommend avoiding relaxed atomic operations unless they’re absolutely necessary, and even then using them only with extreme caution

* One way to achieve additional synchronization without the overhead of full-blown seq_cst is to use *acquire-release ordering*

#### Acquire-Release ordering

* Acquire-release ordering is a step-up from relaxed ordering
    - There's still no total order of operations, but it does introduce some synchronization

* Under this ordering model:
    * Atomic loads are *acquire* operations (`memory_order_acquire`)
    * Atomic stores are *release* operations (`memory_order_release`)
    * Atomic read-modify-write operations (e.g. `fetch_add()` or `exchange()`) are either *acquire*, *release* or both (`memory_order_acq_rel`)

* Synchronization is pairwise between threads that does the release and the thread that does the acquire
    - E.g. *release operation sychronize-with an acquire operation that reads the value written*



# Rust Atomics and Locks

* NOTE: We use this for a brief cleaner overview of the memory model, which is the same in Rust as in C++

## Chapter 3: Memory Ordering


* The memory model is defined in terms of a happens-before relationship
    - That is, it defines when certain operations happen-before another

* Between threads, happens-before relationships only occur in a few specific cases:
    * Spawning and joining a thread
    * Unlocking/Locking a mutex
    * Atomic operations that us non-relaxed memory ordering

* Atomic with relaxed memory order do not guarantee happens-bore, but they guarantee:
    * A total modification order of each individual atomic variable

* The key point is that the memory model is purely based on synchronization
    - So basically threads need to pull new data, rather than it being broadcasted (in MPI)
    - The analogy is saying "Lunch at 12" in a conference room (MPI) versus:
        - Putting a note that says "Lunch at 12" on a bulletin board in the hall
            * People need to go to the hall (perform a happens-before op) to know lunch is at 12

* Relaxed ordering guarantees the total modification order of the same atomic variable happens in order that is the same from the perspective of every single thread
