* Today we are implementing `SharedPtr` and `UniquePtr`
    * To prepare we do a brief reading of:
        1. `Effective Modern C++` Chapter 4
        2. `C++ Concurrency in Action - Williams` Chapter 5.2
        3. **CppReference:** `std::atomic`, `memory_order_relaxed`.

# Chapter 4: Smart Pointers

* Why raw pointers are hard to love:
    1. Declaration doesn't state type -- single object or array
    2. Reveals nothing about whether you destroy what it points to (e.g. if the pointer owns the thing)
        - As we saw in vector::emplace_back()!
    3. No way to tell *how* to destroy, if you're supposed to
    4. Hard to ensure you destroy everything exactly once and not more
    5. Typically no way to tell if the pointer dangles!

* Raw pointers **are powerful tools**, but decades of experience shows with only a **slight lapse in concentration, these tools turn on their masters**
    - Smart pointers to the rescue!

## Item 18: Use `std::unique_ptr` for exclusive-ownership resource management

* Factory functions are a common use case for `std::unique_ptr`

* If a raw pointer is small enough and fast enough for you, `unique_ptr` almost certainly is too
    - Very little extra overhead

* By default, destruction is performed using `delete`
    - But during construction, `std::unique_ptr` can be configured to use custom deleters
    - Custom deleters can force the size of unique_ptr to grow
        * Stateless function objects (e.g. from lambdas with no captures) typically incur no size penalty

* unique_ptr<T[]> also exists, but it should only be of intellectual curiosity
    - Vector, array, etc are virtually always the better data structures

* You can readily convert a unique_ptr to a shared_ptr:
    `std::shared_ptr<Investment> sp = makeInvestment ( arguments);`
    -- where `makeInvestment` returns a unique_ptr


## Item 19: Use `std::shared_ptr` for shared-ownership resource management

* A shared_ptr can tell whether it's the last one by consulting the resources *reference count*
    - Shared_ptr are twice the size, containing:
        - A raw pointer
        - A pointer to the ref count

* Memory for the reference count must be dynamically allocated

* **Increments and decrements of the reference count must be atomic!**
    * Because of simultaneous readers and writers in different threads
    * Therefore reads/writes are costly!

* Hence moving shared_ptrs is faster than copying due to not having to increment the counter

* Unlike unique_ptr, we do not need to decl the type for a custom deleter!
    - This is valid:
        `std::shared_ptr<Widget> spw(new Widget, loggingDel);`
    - Therefore, you can have:
   ```C++
        std::shared_ptr<Widget> pw1(new Widget, customDeleter1);
        std::shared_ptr<Widget> pw2(new Widget, customDeleter2);
        std::vector<std::shared_ptr<Widget>> vpw{ pw1, pw2 };
    ```
    - This custom deleter also doesn't use any more memory in the shared_ptr object itself!
        * `shared_ptr` actually stores the:
            * reference count
            * weak count
            * other data ( custom deleter, allocator, etc)
        all in a **control block** data structure on the heap

* In general, its impossible for a function creating a `shared_ptr` to an object to know whether some other `shared_ptr` already points to that object!
    - Therefore, the following rules for control block  creation are used:
        1. `std::make_shared` always creates a control block
            - There is no control block for that object at the time `make_shared` is called
        2. unique_ptr -> shared_ptr generates a control block
        3. When `std::shared_ptr` constructor is called with a raw pointer, it creates a control block
            - Creating more than one shared_ptr from a raw pointer leads to undefined behavior! Extremely bad code!

* **Try to avoid passing raw pointers to `std::shared_ptr` constructor**
    - Instead use `std::make_shared`
    - If you absolutely **must**, use shared_ptr, then pass the result of new directly:
        `std::shared_ptr<Widget> spw1(new Widget, loggingDel);`

* An example of dangerous code for shared_ptr:
```
class Widget;

// A global list of widgets that have been "processed"
std::vector<std::shared_ptr<Widget>> processedWidgets;

class Widget {
public:
    void process() {
        // ... perform processing logic ...

        // THE BUG:
        // 'this' is a raw pointer.
        // Passing 'this' into the shared_ptr constructor here tells
        // the vector: "I am a brand new object, please create a
        // brand new control block for me."
        processedWidgets.emplace_back(this);
    }
};

int main() {
    // 1. Control Block A is created (Ref count = 1)
    auto spw = std::make_shared<Widget>();

    // 2. Inside process(), Control Block B is created for the SAME Widget
    spw->process();

}
```
    - In doing `.emplace_back(this)`, we are casting a pointer of Widget (this) to a shared_ptr, but we already had a shared_ptr that we started the process with!
        * We created to copies!
    - STL has a way to get around this by using:
    ```
    class Widget: public std::enable_shared_from_this<Widget> {
    public:
        ...
        }
    ```
        * `enable_shared_from_this` is a base class that has a function that generated `shared_ptr` without duplicating control blocks
        * It is used via:
            ```
             void Widget::process()
            {
            // as before, process the Widget
            ...
            // add std::shared_ptr to current object to processedWidgets
            processedWidgets.emplace_back(shared_from_this());
            }
            ```
            - This works by finding the control block associated with the pointer
            - **This gives undefined behavior if there is no shared ptr**!


* Though the control block is typically only a few words, the usual implementation is more sophisticated than you might think
    - Typically using inheritance and a virtual function (and the associated cost!)
    - Though it sounds expensive, (with default deleters and allocators) it is nearly the same cost as standard pointers and the cost of one or two atomic operations

## Item 21: Prefer `std::make_unique` and `std::make_shared` to direct use of `new`

* Small note, `make_unique` was introduced in C++14, but `make_shared` in C++11

* `make_shared` only requires one memory allocation because it can hold both the object and the control block in the same memory allocation
    - Contrasted with `std::shared_ptr<Widget> spw(new Widget);` which requires two

* This guideline is to **prefer** make, but not rely on them exclusively
    - For instance, `make_*` cannot specify custom destructors, where it's easy for using new:
        ```
        std::unique_ptr<Widget, decltype(widgetDeleter)> upw(new Widget, widgetDeleter);
        std::shared_ptr<Widget> spw(new Widget, widgetDeleter);
        ```

    - The initialisation also always sends it via parentheses and not braces, so:
            ```
            auto upv = std::make_unique<std::vector<int>>(10, 20);
            auto spv = std::make_shared<std::vector<int>>(10, 20);
            ```
        generates a vector of length 10 with 20's

* You need to be careful passing my lvalue for shared_ptr because that performs a copy and an atomic increment!
    - Make use of the move when you must pass in the shared_ptr and cannot use make_shared
