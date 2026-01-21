#pragma once

#include <cstddef>          // size_t
#include <utility>          // std::move, std::forward
#include <new>              // placement new
#include <cassert>          // assert
#include <stdexcept>        // std::out_of_range
#include <initializer_list> // std::initializer_list

namespace My {

template <typename T>
class Vector {
public:
    // Standard typedefs
    using iterator = T*;
    using const_iterator = const T*;
    using value_type = T;
    using size_type = size_t;

    // --- Constructors / Destructor ---
    Vector(): data_(nullptr), size_(0), capacity_(0) {};
    ~Vector(){
        //Reverse order is C++ convention
        for (size_t i = size_; i > 0; i--) {
            data_[i-1].~T();
        }

        ::operator delete(data_);
    };

    // Disable Copy (HFT Strictness)
    Vector(const Vector&) = delete;
    Vector& operator=(const Vector&) = delete;

    // Move Semantics (Required)
    Vector(Vector&& other) noexcept;
    Vector& operator=(Vector&& other) noexcept;

    // --- Core Memory Operations ---

    // Allocates raw memory. Moves elements if resizing.
    void reserve(size_t new_cap) {

        if (new_cap <= capacity_) return;

        //1) Allocate new memory
        // new operator
        T* new_data = static_cast<T*>(::operator new(new_cap*sizeof(T)));


        //2) Move old data
        // placement new (with move commands)
        for (size_t i=0; i < size_; i++) {
            new (new_data + i) T(std::move(data_[i]));

            //Release the old data
            data_[i].~T();
        }

        //3) Destroy and free old data location
        ::operator delete(data_);

        data_ = new_data;
        capacity_ = new_cap;
    };

    // Constructs element in-place. Handles growth if needed.
    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            //Expand
            size_t new_cap = (capacity_ == 0) ? 2 : capacity_*2;
            reserve(new_cap);
        }

        //Placement new
        new (data_ + size_) T(std::forward<Args>(args)...);

        T& ref = data_[size_];
        size_++;

        return ref;
    };


    // Wrappers (Implement these using emplace_back)
    void push_back(const T& value) {
        emplace_back(value);
    };
    void push_back(T&& value) {
        emplace_back(std::move(value));
    };

    void pop_back() {
        if (size_ > 0) {
            data_[size_-1].~T();
            size_--;
        }
    };
    void clear(){
        for (size_t i=size_; i > 0; i--) {
            data_[i-1].~T();
        }
        //Reset size, but keep capacity
        size_ = 0;
    };


    // --- Accessors ---
    T* data() {
        return &data_[0];
    };
    const T* data() const {
        return &data_[0];
    };

    size_t size() const { return size_;};
    size_t capacity() const { return capacity_;};
    bool empty() const {return size_ == 0;};

    T& operator[](size_t index) {
        assert(index < size_);
        return data_[index];
    };
    const T& operator[](size_t index) const {
        assert(index < size_);
        return data_[index];
    };

    T& at(size_t index){
        if (index >= size_){
            throw std::out_of_range("Vector::at -- Index out of range");
        }
        return data_[index];
    };

    const T& at(size_t index) const
    {
        if (index >= size_){
            throw std::out_of_range("Vector::at -- Index out of range");
        }
        return data_[index];
    };

    T& front() {
        return data_[0];
    };
    const T& front() const {
        return data_[0];
    };
    T& back() {
        return data_[size_-1];
    };
    const T& back() const {
        return data_[size_-1];
    };

    // --- Iterators ---
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

} // namespace My

