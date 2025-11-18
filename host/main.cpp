#include "../utils/algebra.h"
#include "../utils/general.h"
#include <iostream>
#include <array>

using algebra::Vector;
using std::array;

int main() {
    std::cout << "Host harness for utils" << std::endl;

    Vector<int, 3> a (array<int, 3>{ 1, 2, 3 });
    Vector<int, 3> b (array<int, 3>{ -2, 4, 0 });

    auto c = a + b;

    println("Size check: ", c.size());
    println("Element check: ", c[0], ", ", c[1], ", ", c[2]);

    Vector<int, 3> moved = std::move(c);
    println("Move check, size after move: ", moved.size());

    return 0;
}