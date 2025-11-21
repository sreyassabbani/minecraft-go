#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <ST7796S.h>
#include <cstdint>
#include <cstring>

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
    Display(ST7796S& driver, uint16_t width = ST7796S_TFTWIDTH,
            uint16_t height = ST7796S_TFTHEIGHT)
        : tft(driver), width_(width), height_(height) {}

    void begin(uint8_t rotation = 1, Color clearColor = Color::Black(),
               bool textWrap = true, uint32_t spiFreq = 0) {
        tft.begin(spiFreq);
        tft.setRotation(rotation);
        tft.setTextWrap(textWrap);
        tft.fillScreen(clearColor.value);
    }

    void clear(Color color) { tft.fillScreen(color.value); }

    void drawPixel(int16_t x, int16_t y, Color color) {
        tft.drawPixel(x, y, color.value);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color) {
        tft.fillRect(x, y, w, h, color.value);
    }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color) {
        tft.drawRect(x, y, w, h, color.value);
    }

    void drawFastHLine(int16_t x, int16_t y, int16_t w, Color color) {
        tft.drawFastHLine(x, y, w, color.value);
    }

    void drawFastVLine(int16_t x, int16_t y, int16_t h, Color color) {
        tft.drawFastVLine(x, y, h, color.value);
    }

    void drawText(int16_t x, int16_t y, const char* text,
                  const TextStyle& style) {
        tft.setCursor(x, y);
        tft.setTextColor(style.fg.value, style.bg.value);
        tft.setTextSize(style.size);
        tft.print(text);
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

    void blit565(int16_t x, int16_t y, const uint16_t* pixels16,
                 int16_t w, int16_t h) {
        tft.drawRGBBitmap(x, y, pixels16, w, h);
    }

    uint16_t width() const { return width_; }
    uint16_t height() const { return height_; }

private:
    ST7796S& tft;
    uint16_t width_;
    uint16_t height_;
};

} // namespace display

#endif // DISPLAY_HPP
