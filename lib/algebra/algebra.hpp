#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
using std::array;

namespace algebra {

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

template <size_t LENGTH> class Vector {
    array<float, LENGTH> data {};

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

    // Multiply by a 4x4 matrix; for LENGTH 3, assumes w=1
    Vector<4> operator*(const Matrix<4, 4>& m) const {
        static_assert(LENGTH == 3 || LENGTH == 4,
                      "Vector * Matrix only supported for length 3 or 4");
        const float x = data[0];
        const float y = data[1];
        const float z = data[2];
        const float w = (LENGTH == 3) ? 1.0f : data[3];
        return Vector<4> {
            x * m(0, 0) + y * m(1, 0) + z * m(2, 0) + w * m(3, 0),
            x * m(0, 1) + y * m(1, 1) + z * m(2, 1) + w * m(3, 1),
            x * m(0, 2) + y * m(1, 2) + z * m(2, 2) + w * m(3, 2),
            x * m(0, 3) + y * m(1, 3) + z * m(2, 3) + w * m(3, 3)
        };
    }
};

struct Quaternion {
    float w, x, y, z;
};

Vector<3> rotateVector(const Quaternion& q, const Vector<3>& v);
Vector<3> quaternionToEuler(const Quaternion& q);
Quaternion normalizeQuaternion(const Quaternion& q);
Vector<3> cross(const Vector<3>& a, const Vector<3>& b);
float length(const Vector<3>& v);
Vector<3> normalize(const Vector<3>& v);
Vector<3> normalizeOr(const Vector<3>& v, const Vector<3>& fallback);
Vector<3> faceNormal(const Vector<3>& v0, const Vector<3>& v1,
                     const Vector<3>& v2);
Quaternion makeYawPitch(float yaw, float pitch);
Quaternion lookAt(const Vector<3>& pos, const Vector<3>& target);

} // namespace algebra
