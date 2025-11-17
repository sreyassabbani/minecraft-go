template<typename T>
void _println_impl(const T& value);

template<typename T, typename... Args>
void _println_impl(const T& value, const Args&... args);

template<typename... Args>
void println(const Args&... args);