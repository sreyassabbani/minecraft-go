#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <ST7796S.h>
#include <cstdint>
#include <cstring>
#include <new>

namespace display {

struct Color {
    uint16_t value;

    constexpr Color() : value(0) {}
    constexpr explicit Color(uint16_t v) : value(v) {}

    static constexpr Color from565(uint16_t v) { return Color(v); }
    static constexpr Color fromRGB(uint8_t r, uint8_t g, uint8_t b) {
        return Color(static_cast<uint16_t>(((r & 0xF8) << 8) |
                                           ((g & 0xFC) << 3) | (b >> 3)));
    }
    static constexpr Color Black() { return Color { 0x0000 }; }
    static constexpr Color White() { return Color { 0xFFFF }; }
    static constexpr Color Red() { return Color { 0xF800 }; }
    static constexpr Color Green() { return Color { 0x07E0 }; }
    static constexpr Color Blue() { return Color { 0x001F }; }
    // Minecraft sky tint (more saturated so it doesn't wash out to white)
    static constexpr Color SkyBlue() { return fromRGB(120, 170, 255); }
    static constexpr Color Gray() { return Color { 0x8410 }; }
};

struct TextStyle {
    Color fg;
    Color bg;
    uint8_t size;

    constexpr TextStyle() : fg(Color::White()), bg(Color::Black()), size(1) {}
    constexpr TextStyle(Color fgColor, Color bgColor, uint8_t textSize = 1)
        : fg(fgColor), bg(bgColor), size(textSize) {}
};

class Display {
public:
    struct Pins {
        int8_t cs;
        int8_t dc;
        int8_t rst;
    };

    static constexpr uint32_t defaultSpiHz() { return 42000000UL; }

    explicit Display(uint8_t rotation = 1, Color clear = Color::Black(),
                     bool wrap = true, uint32_t spiFreq = defaultSpiHz()) {
        if (!tft_) {
            tft_ = new (&tftStorage_) ST7796S(pins_.cs, pins_.dc, pins_.rst);
        }
        width_ = ST7796S_TFTWIDTH;
        height_ = ST7796S_TFTHEIGHT;

        tft().begin(spiFreq);
        tft().setRotation(rotation);
        tft().setTextWrap(wrap);
        tft().fillScreen(clear.value);
    }

    void setPins(const Pins& pins) { pins_ = pins; }

    void clear(Color color) { tft().fillScreen(color.value); }

    void drawPixel(int16_t x, int16_t y, Color color) {
        tft().drawPixel(x, y, color.value);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color) {
        tft().fillRect(x, y, w, h, color.value);
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color) {
        tft().drawRect(x, y, w, h, color.value);
    }

    void drawFastHLine(int16_t x, int16_t y, int16_t w, Color color) {
        tft().drawFastHLine(x, y, w, color.value);
    }

    void drawFastVLine(int16_t x, int16_t y, int16_t h, Color color) {
        tft().drawFastVLine(x, y, h, color.value);
    }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Color color) {
        tft().drawLine(x0, y0, x1, y1, color.value);
    }

    void drawText(int16_t x, int16_t y, const char* text,
                  const TextStyle& style) {
        tft().setCursor(x, y);
        tft().setTextColor(style.fg.value, style.bg.value);
        tft().setTextSize(style.size);
        tft().print(text);
    }

    void drawTextCentered(int16_t cx, int16_t cy, const char* text,
                          const TextStyle& style) {
        const size_t len = std::strlen(text);
        const int16_t textW = static_cast<int16_t>(len) * 6 * style.size;
        const int16_t textH = 8 * style.size;
        const int16_t x = cx - textW / 2;
        const int16_t y = cy - textH / 2;
        drawText(x, y, text, style);
    }

    void blit565(int16_t x, int16_t y, const uint16_t* pixels16, int16_t w,
                 int16_t h) {
        tft().drawRGBBitmap(x, y, pixels16, w, h);
    }

    uint16_t width() const { return width_; }
    uint16_t height() const { return height_; }

private:
    ST7796S& tft() { return *tft_; }
    const ST7796S& tft() const { return *tft_; }

    ST7796S* tft_ { nullptr };
    alignas(ST7796S) uint8_t tftStorage_[sizeof(ST7796S)];
    Pins pins_ { 10, 9, 8 }; // defaults; override with setPins()
    uint16_t width_ { 0 };
    uint16_t height_ { 0 };
};

static inline Display& screen() {
    static Display s;
    return s;
}

} // namespace display

#endif // DISPLAY_HPP
