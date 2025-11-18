#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <utility>
#include <vector>
using namespace std;

namespace algebra {

template <typename T> class Vector {
    vector<T> data;

public:
    // not marked `explicit` (`implicit` should have been an opt-in feature)
    Vector(vector<T>& v) : data(v) {}

    // copy constructor (takes a ref to an object of the same class as its
    // argument)
    Vector(const vector<T>& v) : data(v) {}
    Vector(const Vector<T>& other) : data(other.data) {}

    // move constructor (takes a ref to an existing object, an rvalue ref, from
    // whom ownership of resources is transferred). after the function call,
    // `other.data` will reach a valid but unspecified state; that state is
    // typically empty, which is correctly handled by `std::move`
    Vector(Vector<T>&& other) noexcept : data(std::move(other.data)) {}

    size_t size() const { return data.size(); }

    const T& operator[](size_t idx) const {
        assert(idx < data.size());
        return data[idx];
    }

    T& operator[](size_t idx) {
        assert(idx < data.size());
        return data[idx];
    }

    // move assignment operator
    Vector& operator=(Vector<T>&& other) noexcept {
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

} // namespace algebra

#endif
