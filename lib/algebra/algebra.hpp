#ifndef VECTOR_H
#define VECTOR_H

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#if defined(__AVR__) || defined(ARDUINO_ARCH_AVR)
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define ALG_PLAIN_ARRAY
namespace std {
template <class T> class initializer_list {
    const T* begin_;
    size_t size_;

public:
    constexpr initializer_list() : begin_(nullptr), size_(0) {}
    constexpr initializer_list(const T* b, size_t s) : begin_(b), size_(s) {}
    constexpr const T* begin() const { return begin_; }
    constexpr const T* end() const { return begin_ + size_; }
    constexpr size_t size() const { return size_; }
};
} // namespace std
#else
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
using std::array;
#endif

namespace algebra {

template <size_t LENGTH> class Vector {
#ifdef ALG_PLAIN_ARRAY
    float data[LENGTH] {};
#else
    array<float, LENGTH> data {};
#endif

public:
    Vector() = default;
    explicit Vector(const float (&v)[LENGTH]) {
        for (size_t i = 0; i < LENGTH; ++i) data[i] = v[i];
    }
#ifndef ALG_PLAIN_ARRAY
    explicit Vector(const array<float, LENGTH>& v) : data(v) {}
#endif

    Vector(std::initializer_list<float> ilist) {
        assert(ilist.size() == LENGTH);
        size_t i = 0;
        for (float x : ilist) data[i++] = x;
    }

    Vector(const Vector&) = default;
    Vector& operator=(const Vector&) = default;

    Vector(Vector&& other) noexcept = default;

    static constexpr size_t length() noexcept { return LENGTH; }

    float& operator[](size_t idx) {
        assert(idx < LENGTH);
        return data[idx];
    }

    const float& operator[](size_t idx) const {
        assert(idx < LENGTH);
        return data[idx];
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            for (size_t i = 0; i < LENGTH; ++i) data[i] = other.data[i];
        }
        return *this;
    }

    Vector operator+(const Vector& other) const {
        assert(this->length() == other.length());

        Vector res = *this;
        for (size_t i = 0; i < LENGTH; ++i) { res.data[i] += other.data[i]; }
        return res;
    }

    Vector operator-(const Vector& other) const {
        assert(this->length() == other.length());

        Vector res = *this;
        for (size_t i = 0; i < LENGTH; ++i) { res.data[i] -= other.data[i]; }
        return res;
    }

    Vector operator*(float scalar) const {
        Vector res = *this;
        for (size_t i = 0; i < LENGTH; ++i) { res.data[i] *= scalar; }
        return res;
    }

    float dot(const Vector& other) const {
        float res = 0;

        for (size_t i = 0; i < other.length(); ++i) {
            res += other[i] * data[i];
        }

        return res;
    }
};

template <size_t ROWS, size_t COLS> class Matrix {
    float data[ROWS][COLS] {};

public:
    size_t rows() const { return ROWS; }
    size_t cols() const { return COLS; }

    float& operator()(size_t r, size_t c) { return data[r][c]; }
    float operator()(size_t r, size_t c) const { return data[r][c]; }

    template <size_t OCOLS>
    Matrix<ROWS, OCOLS> operator*(Matrix<COLS, OCOLS> const& rhs) const {
        Matrix<ROWS, OCOLS> res {};

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

struct Quaternion {
    float w, x, y, z;
};

Vector<3> rotateVector(const Quaternion& q, const Vector<3>& v);
Vector<3> quaternionToEuler(const Quaternion& q);
Quaternion normalizeQuaternion(const Quaternion& q);

} // namespace algebra

#endif
