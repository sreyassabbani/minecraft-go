#ifndef VECTOR_H
#define VECTOR_H

#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <utility>

using std::array;

namespace algebra {

template <size_t LENGTH> class Vector {
    array<float, LENGTH> data;

public:
    Vector() : data {} {}
    explicit Vector(const array<float, LENGTH>& v) : data(v) {}

    explicit Vector(std::initializer_list<float> ilist) {
        assert(ilist.size() == LENGTH);
        size_t i = 0;
        for (float x : ilist) data[i++] = x;
    }

    Vector(const Vector&) = default;
    Vector& operator=(const Vector&) = default;

    Vector(Vector&& other) noexcept : data(std::move(other.data)) {}

    static constexpr size_t length() noexcept { return LENGTH; }

    float& operator[](size_t idx) {
        assert(idx < data.size());
        return data[idx];
    }

    const float& operator[](size_t idx) const {
        assert(idx < data.size());
        return data[idx];
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) { data = std::move(other.data); }
        return *this;
    }

    Vector operator+(const Vector& other) const {
        assert(this->length() == other.length());

        Vector res = *this;
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

    float dot(const Vector& other) const {
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
