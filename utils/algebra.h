#ifndef VECTOR_H
#define VECTOR_H

#include "general.h"
#include <array>
#include <cassert>
#include <utility>

using std::array;

namespace algebra {

template <size_t LENGTH> class Vector {
    array<float, LENGTH> data;

public:
    // not marked `explicit` (`implicit` should have been an opt-in feature)
    // Vector(array<float, LENGTH> v) : data(v) {}

    // copy constructor (takes a ref to an object of the same class as its
    // argument)
    explicit Vector(const array<float, LENGTH>& v) : data(v) {}
    // Vector(const Vector<LENGTH>& other) : data(other.data) {}

    Vector(const Vector&) = default;
    Vector& operator=(const Vector&) = default;

    // move constructor (takes a ref to an existing object, an rvalue ref, from
    // whom ownership of resources is transferred). after the function call,
    // `other.data` will reach a valid but unspecified state; that state is
    // typically empty, which is correctly handled by `std::move`
    Vector(Vector&& other) noexcept : data(std::move(other.data)) {}

    // `const` and/or `noexcept` and/or `constexpr` and/or `static`?? also
    // aren't some of these inferred by the compiler
    static constexpr size_t length() noexcept { return LENGTH; }

    // operator[]: non-const and const overloads
    float& operator[](size_t idx) {
        assert(idx < data.size());
        return data[idx];
    }

    const float& operator[](size_t idx) const {
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
        assert(this->length() == other.length());

        // copy construction of the result
        Vector res = *this; // call the copy constructor!
        for (size_t i = 0; i < res.data.size(); ++i) {
            res.data[i] += other.data[i];
        }
        return res;
    }

    Vector operator-(const Vector& other) const {
        assert(this->length() == other.length());

        Vector res = *this;
        for (size_t i = 0; i < res.data.size(); ++i) {
            res.data[i] -= other.data[i];
        }
        return res;
    }

    float dot(Vector& other) const {
        float res = 0;

        for (size_t i = 0; i < other.length(); ++i) {
            res += other[i] * data[i];
        }

        return res;
    }
};

template <size_t ROWS, size_t COLS> class Matrix {
    array<array<float, COLS>, ROWS> data;

public:
    size_t rows() const { return ROWS; }
    size_t cols() const { return COLS; }

    float& operator()(size_t r, size_t c) { return data[r][c]; }
    float operator()(size_t r, size_t c) const { return data[r][c]; }

    template <size_t OCOLS>
    Matrix<ROWS, OCOLS> operator*(Matrix<COLS, OCOLS> const& rhs) const {
        Matrix<ROWS, OCOLS> res {};

        println(res);

        for (size_t i = 0; i < ROWS; ++i) {
            for (size_t j = 0; j < OCOLS; ++j) {
                float sum = 0.0f;
                for (size_t k = 0; k < COLS; ++k) {
                    sum += data[i][k] * rhs.data[k][j];
                }
                res(i, j) = sum;
            }
        }

        return res;
    }
};

} // namespace algebra

#endif
