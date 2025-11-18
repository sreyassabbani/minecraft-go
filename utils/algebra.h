#ifndef VECTOR_H
#define VECTOR_H

#include <vector>
#include <cassert>
#include <utility>
using namespace std;

namespace algebra {

template <typename T>
class Vector {
    vector<T> data;
public:
    Vector(vector<T>& v);

    Vector(const vector<T>& v);

    Vector(const Vector<T>& other);

    Vector(Vector<T>&& other) noexcept;

    size_t size() const;

    Vector& operator=(Vector<T>&& other) noexcept;

    Vector operator+(const Vector& other) const;
};

}

#endif