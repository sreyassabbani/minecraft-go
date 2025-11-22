# Display

We use an `ST7796S` 4.0'' TFT SPI `480x320` LCD for this project. We have abstracted the `Adafruit_ST7796S_kbv` library with a `display::Display` class for our purposes. To get started with a display, simply instantiate and run an initializing function:

```cpp
#include <display.hpp>

display::Display screen;

void setup() {
    screen.begin();
}
```

You can look at the source code for more information on optional parameters that you may pass to `Display::begin`.
