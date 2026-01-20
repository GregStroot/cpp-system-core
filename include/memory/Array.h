#pragma once
#include <cassert>          // Assert
#include <cstddef>           // size_t
#include <stdexcept>        // Out of range
#include <initializer_list> // std::initializer_list
#include <iterator>        // std::reverse_iterator

namespace My {

    template<typename T, size_t N>
        class Array {
            public:
                using iterator = T*;
                using const_iterator = const T*;
                using reference = T&;
                using const_reference = const T&;
                using value_type = T;
                using size_type = size_t;

                // 2.Constructors
                constexpr Array() = default;
                constexpr Array(std::initializer_list<T> list) {

                    auto it = list.begin();
                    for (size_t i=0; i < N; i++) {

                        if (it == list.end()) {
                            data_[i] == T();
                        }
                        else {
                            data_[i] = *it;
                            it++;
                        }

                    }
                }

                constexpr void fill(const T& val) {
                    for (size_t i=0; i < N; i++){
                        data_[i] = val;
                    }
                }

                // 3. Accessors
                constexpr reference operator[](size_t index) {

                    assert(index < N && "Index is out of range of array");

                    return data_[index];
                }

                // Requirement: Use assert(index < N) for debug safety.
                constexpr const_reference operator[](size_t index) const {
                    assert(index < N && "Index is out of range of array");

                    return data_[index];
                }

                // Return the raw pointer to the underlying array.
                constexpr iterator data() {
                    return &data_[0];
                }

                constexpr const_iterator data() const {
                    // [YOUR CODE HERE]
                    return &data_[0];
                }

                constexpr reference front() {
                    return data_[0];
                }

                constexpr const_reference front() const {
                    return data_[0];
                }

                constexpr reference back() {
                    return data_[N-1];
                }

                constexpr const_reference back() const {
                    return data_[N-1];
                }

                constexpr reference at(size_t index) {
                    if (index >= N) {
                        throw std::out_of_range("Array::at - Index out of bounds");
                    }
                    return data_[index];
                }

                constexpr const_reference at(size_t index) const {
                    if (index >= N) {
                        throw std::out_of_range("Array::at - Index out of bounds");
                    }
                    return data_[index];
                }



                // 4. Capacity
                constexpr size_t size() const { return N; }
                constexpr size_t empty() const { return N==0;}

                // 5. Iterators
                constexpr iterator begin() {
                    return &data_[0];
                }

                constexpr iterator end() {
                    return &data_[0] + N;
                }

                constexpr const_iterator begin() const {
                    return &data_[0];
                }

                constexpr const_iterator end() const {
                    // [YOUR CODE HERE]
                    return &data_[0] + N;
                }


            private:
                T data_[N];
        };
}

