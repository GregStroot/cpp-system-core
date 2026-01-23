#pragma once
#include <utility> // For std::move, std::forward
#include <cstddef> // For std::nullptr_t

namespace My {

    template<typename T>
    class UniquePtr {
    public:
        // --- Constructors ---

        // Default Constructor
        constexpr UniquePtr() noexcept: ptr_(nullptr) {};

        // Constructor from raw pointer
        explicit UniquePtr(T* ptr) noexcept: ptr_(ptr) {};

        // Constructor from nullptr
        constexpr UniquePtr(std::nullptr_t) noexcept: ptr_(nullptr) {};

        // --- Destructor ---
        ~UniquePtr() {
            delete ptr_;
        };

        // --- No Copying Allowed (Exclusive Ownership) ---
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        // --- Move Semantics (Transfer Ownership) ---
        UniquePtr(UniquePtr&& other) noexcept {
            ptr_ = other.ptr_;
            //Prevent double clean-up!
            other.ptr_ = nullptr;
        };
        UniquePtr& operator=(UniquePtr&& other) noexcept
        {
            //Check that we aren't assinging to self
            if (this == &other) {
                return *this;
            }
            //Throw away old data
            delete ptr_;
            //Reassign new data
            ptr_ = other.ptr_;
            //Prevent double clean-up
            other.ptr_ = nullptr;
            return *this;
        };

        // --- Observers ---
        T* get() const noexcept{
            return ptr_;
        };
        T& operator*() const {
            return *ptr_;
        };
        T* operator->() const noexcept {
            return ptr_;
        };
        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        };

        // --- Modifiers ---

        // Releases ownership of the pointer (does NOT destroy it). Returns the raw pointer.
        T* release() noexcept {
            T* newPtr = ptr_;
            ptr_ = nullptr;
            return newPtr;
        };

        // Replaces the managed object. Destroys the old object (if any).
        void reset(T* ptr = nullptr) noexcept {
            if (ptr == ptr_) {
                return;
            }
            delete ptr_;
            ptr_ = ptr;
        };

    private:
        T* ptr_ = nullptr;
    };

    // --- Helper: make_unique ---
    // (Optional Challenge: Try to implement this without looking up the syntax if you can)
    template<typename T, typename... Args>
    UniquePtr<T> make_unique(Args&&... args) {
        //Create new with forwarded arguments

        T* ptr = new T(std::forward<Args>(args)...);

        //Return constructed unique_ptr
        return UniquePtr(ptr);
    };

}

