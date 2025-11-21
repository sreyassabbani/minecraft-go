#ifndef GENERAL_H
#define GENERAL_H

#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <iostream>

  struct SerialShim {
      template <typename T>
      void print(const T& value) { std::cout << value; }
      void println() { std::cout << std::endl; }
  };

  inline SerialShim Serial;  // single ODR-safe definition in header
#endif

template <typename T>
void _println_impl(const T& value) { Serial.print(value); }

template <typename T, typename... Args>
void _println_impl(const T& value, const Args&... args) {
    Serial.print(value);
    _println_impl(args...);
}

template <typename... Args>
void println(const Args&... args) {
    _println_impl(args...);
    Serial.println();
}

#endif // GENERAL_H
