#include "../utils/algebra.h"
#include "../utils/general.h"
#include <iostream>

using algebra::Vector;

int main() {
    std::cout << "Host harness for utils" << std::endl;

    Vector<int> a({ 1, 2, 3 });
    Vector<int> b({ -2, 4, 0 });
    auto c = a + b;

    println("Size check: ", c.size());
    println("Element check: ", c[0], ", ", c[1], ", ", c[2]);

    Vector<int> moved = std::move(c);
    println("Move check, size after move: ", moved.size());

    return 0;
}
