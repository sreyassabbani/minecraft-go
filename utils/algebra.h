#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <utility>
#include <array>

using std::array;

namespace algebra {

template <typename T, size_t LENGTH> class Vector {
    array<T, LENGTH> data;

public:
    // not marked `explicit` (`implicit` should have been an opt-in feature)
    // Vector(array<T, LENGTH> v) : data(v) {}

    // copy constructor (takes a ref to an object of the same class as its
    // argument)
    explicit Vector(const array<T, LENGTH>& v) : data(v) {}
    // Vector(const Vector<T, LENGTH>& other) : data(other.data) {}

    
    Vector(const Vector&) = default;
    Vector& operator=(const Vector&) = default;

    // move constructor (takes a ref to an existing object, an rvalue ref, from
    // whom ownership of resources is transferred). after the function call,
    // `other.data` will reach a valid but unspecified state; that state is
    // typically empty, which is correctly handled by `std::move`
    Vector(Vector&& other) noexcept : data(std::move(other.data)) {}

    // `const` and/or `noexcept` and/or `constexpr` and/or `static`?? also aren't some of these inferred by the compiler
    static constexpr size_t size() noexcept { return LENGTH; }
    
    // operator[]: non-const and const overloads
    T& operator[](size_t idx) {
        assert(idx < data.size());
        return data[idx];
    }

    const T& operator[](size_t idx) const {
        assert(idx < data.size());
        return data[idx];
    }

    // move assignment operator
    Vector& operator=(Vector&& other) noexcept {
        // guard against self-move assignment
        if (this != &other) { data = std::move(other.data); }
        return *this;
    }

    Vector operator+(const Vector& other) const {
        // same as `assert(this->data.size() == other.size())` and even
        // `assert(data.size() == other.size())`
        assert(this->size() == other.size());

        // copy construction of the result
        Vector res = *this; // call the copy constructor!
        for (size_t i = 0; i < res.data.size(); ++i) {
            res.data[i] += other.data[i];
        }
        return res;
    }
};

template <typename T, size_t ROWS, size_t COLS>
class Matrix {
    array<T, ROWS * COLS> data;
public:
    size_t rows() {
        return ROWS;
    }

    size_t cols() {
        return COLS;
    }
};

} // namespace algebra

#endif
