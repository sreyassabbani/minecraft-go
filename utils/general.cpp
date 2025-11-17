template<typename T>
void _println_impl(const T& value) {
  Serial.print(value);
}

template<typename T, typename... Args>
void _println_impl(const T& value, const Args&... args) {
  Serial.print(value);
  _println_impl(args...);
}

template<typename... Args>
void println(const Args&... args) {
  _println_impl(args...);
  Serial.println();
}