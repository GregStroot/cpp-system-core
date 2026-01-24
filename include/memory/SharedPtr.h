#pragma once
#include <cstddef>  // std::nullptr_t
#include <utility>  // std::forward, std::move
#include <atomic>   // std::atomic

namespace My {

    // --- Forward Declaration of Control Block ---
    // You must implement the Control Block hierarchy in your private/implementation section.
    // It is not exposed to the user.
    struct ControlBlock {
        std::atomic<long> refCount = 1;

        //Universal button
        virtual void dispose() = 0;

        //Virtual destructor
        virtual ~ControlBlock() = default;
    };

    template<typename T>
    struct DefaultControlBlock : ControlBlock {
        T* managed_ptr_;

        DefaultControlBlock(T* ptr) : managed_ptr_(ptr) {};

        void dispose() override {
            delete managed_ptr_;
            delete this;
        };
    };

    template<typename T>
    struct AllocationBlock : ControlBlock {
        T object;

        template<typename... Args>
        AllocationBlock(Args&&... args):
            object(std::forward<Args>(args)...)
        {};

        void dispose() override {
            delete this;
        }
    };



    template<typename T>
    class SharedPtr {
    public:
        // --- Constructors ---

        // Default Constructor (Null)
        constexpr SharedPtr() noexcept {
            cb_ = nullptr;
            ptr_ = nullptr;
        };
        constexpr SharedPtr(std::nullptr_t) noexcept;

        // Constructor from raw pointer
        // Requirement: Allocates a Control Block.
        explicit SharedPtr(T* ptr) {
            cb_ = new My::DefaultControlBlock<T>(ptr);
            ptr_ = ptr;
        };


        // Copy Constructor
        // Requirement: Increments ref_count.
        SharedPtr(const SharedPtr& other) noexcept {
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            if (cb_) {
                cb_->refCount++;
            }
        };

        // Move Constructor
        // Requirement: Steals ownership. Ref_count remains unchanged.
        SharedPtr(SharedPtr&& other) noexcept {
            cb_ = other.cb_;
            ptr_ = other.ptr_;

            other.cb_ = nullptr;
            other.ptr_ = nullptr;
        };

        // --- Destructor ---
        // Requirement: Decrements ref_count. If 0, destroys object AND Control Block.
        ~SharedPtr() {
            release_ownership();
        };

        // --- Assignment Operators ---

        // Copy Assignment
        SharedPtr& operator=(const SharedPtr& other) noexcept {
            if (other.cb_) {
                other.cb_->refCount++;
            }

            release_ownership();

            cb_ = other.cb_;
            ptr_ = other.ptr_;

            return *this;
        };

        // Move Assignment
        SharedPtr& operator=(SharedPtr&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            release_ownership();


            cb_ = other.cb_;
            ptr_ = other.ptr_;

            other.cb_ = nullptr;
            other.ptr_ = nullptr;

            return *this;

        };

        // --- Modifiers ---

        // Replaces the managed object.
        void reset() noexcept {
            release_ownership();
            ptr_ = nullptr;
            cb_ = nullptr;
        };
        void reset(T* ptr) {
            if (ptr == ptr_) {
                return;
            }

            DefaultControlBlock<T>* temp_cb = nullptr;
            if (ptr) {
                temp_cb = new DefaultControlBlock<T>(ptr);
            }

            release_ownership();

            ptr_ = ptr;
            cb_ = temp_cb;
        };

        // --- Observers ---

        T* get() const noexcept {
            return ptr_;
        };
        T& operator*() const noexcept {
            return *ptr_;
        };
        T* operator->() const noexcept {
            return ptr_;
        };

        // Returns the current number of owners.
        long use_count() const noexcept {
            return (cb_ != nullptr) ? cb_->refCount.load() : 0;
        };

        explicit operator bool() const noexcept {
            return (ptr_ != nullptr) ? true: false;
        };

    private:
        // Constructor from raw pointer and cb
        // This is dangerous to allow a user to use it
        explicit SharedPtr(T* ptr, ControlBlock* cb) {
            ptr_ = ptr;
            cb_ = cb;
        };

        // Hint: You need access to the control block for make_shared logic
        template<typename U, typename... Args>
        friend SharedPtr<U> make_shared(Args&&... args);

        void release_ownership() noexcept {
            if (cb_) {
                if (cb_->refCount.fetch_sub(1) == 1) {
                    cb_->dispose();
                }
            }
        };

        ControlBlock* cb_ = nullptr;
        T* ptr_= nullptr;
    };

    // --- Factory Function ---
    // Requirement: Performs ONE memory allocation for both the Object and the Control Block.
    template<typename T, typename... Args>
    SharedPtr<T> make_shared(Args&&... args) {

        AllocationBlock<T>* ac = new AllocationBlock<T>(std::forward<Args>(args)...);

        return SharedPtr(&ac->object, ac);
    };

}

