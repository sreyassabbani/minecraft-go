#include <vector>
#include <cassert>
using namespace std;
namespace algebra {
template <typename T>
class Vector {
    vector<T> data;
public:
    // not marked `explicit` (`implicit` should have been an opt-in feature)
    Vector(vector<T>& v) : data(v.data) {}

    // copy constructor (takes a ref to an object of the same class as its argument)
    Vector(const vector<T>& v) : data(v) {}
    Vector(const Vector<T>& other) : data(other.data) {}

    // move constructor (takes a ref to an existing object, an rvalue ref, from whom ownership of resources is transferred). after the function call, `other.data` will reach a valid but unspecified state; that state is typically empty, which is correctly handled by `std::move`
    Vector(Vector<T>&& other) noexcept : data(move(other.data)) {}

    size_t size() const {
        return data.size();
    }

    // move assignment operator
    Vector& operator=(Vector<T>&& other) noexcept {
        // guard against self-move assignment
        if (this != &other) {
            data = move(other.data);
        }
        return *this;
    }

    Vector& operator+(const Vector& other) const {
        // same as `assert(this->data.size() == other.size())` and even `assert(data.size() == other.size())`
        assert(this->size() == other.size());

        // the following is not copy assignment (which would be for mutation)
        // it is a direct construction/initialization (the parens themselves indicate constructors though).
        // 
        // copy initialization (which calls the copy constructor) is:
        // `vector<T> res = this->data`
        // while in most practicality, the compiler optimizes both to the same thing due to copy elision
        Vector res = *this; // call the copy constructor!
        for (size_t i = 0; i < res.data.size(); ++i) {
            res.data[i] += other.data[i];
        }
        return res;
    }
};
}