#ifndef GENERAL_H
#define GENERAL_H

#include <cstdlib>

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <iostream>

struct SerialShim {
    template <typename T> void print(const T& value) { std::cout << value; }
    template <typename T> void println(const T& value) {
        std::cout << value << std::endl;
    }
    void println() { std::cout << std::endl; }
};

inline SerialShim Serial; // single ODR-safe definition in header
#endif

template <typename T> void _println_impl(const T& value) {
    Serial.print(value);
}

template <typename T, typename... Args>
void _println_impl(const T& value, const Args&... args) {
    Serial.print(value);
    Serial.print(" ");
    _println_impl(args...);
}

template <typename... Args> void println(const Args&... args) {
    _println_impl(args...);
    Serial.println();
}

inline void require(bool ok, const char* msg) {
    if (!ok) {
        println(msg);
        abort();
    }
};

inline void require(bool ok, const char* er_msg, const char* ok_msg) {
    if (!ok) {
        println(er_msg);
        abort();
    } else println(ok_msg);
};

#endif // GENERAL_H
