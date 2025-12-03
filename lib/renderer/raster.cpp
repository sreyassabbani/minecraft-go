#include <raster.hpp>

// Undefine Arduino min/max macros to avoid conflicts with std::min/max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>

namespace Raster {

void fillTriangle(display::Display& display, int16_t x0, int16_t y0, int16_t x1,
                  int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // Sort vertices by Y coordinate (y0 <= y1 <= y2)
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y0 > y2) {
        std::swap(y0, y2);
        std::swap(x0, x2);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }

    // Check for degenerate cases
    if (y0 == y2) return; // All points on same line

    // Check if triangle is flat-bottomed or flat-topped
    if (y1 == y2) {
        // Flat bottom
        fillFlatBottomTriangle(display, x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Flat top
        fillFlatTopTriangle(display, x0, y0, x1, y1, x2, y2, color);
    } else {
        // General case: split into flat-bottom and flat-top
        // Find the point on edge v0-v2 that has same y as v1
        int16_t x3 = x0 + (int16_t)(((int32_t)(y1 - y0) * (int32_t)(x2 - x0)) /
                                    (int32_t)(y2 - y0));
        int16_t y3 = y1;

        fillFlatBottomTriangle(display, x0, y0, x1, y1, x3, y3, color);
        fillFlatTopTriangle(display, x1, y1, x3, y3, x2, y2, color);
    }
}

void fillFlatBottomTriangle(display::Display& display, int16_t x0, int16_t y0,
                            int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                            uint16_t color) {
    // v0 is top vertex, v1 and v2 are bottom (y1 == y2)
    if (y1 <= y0) return; // Degenerate

    // Calculate inverse slopes (dx/dy) using fixed-point
    // Using 16.16 fixed point for sub-pixel accuracy
    int32_t invslope1 = ((int32_t)(x1 - x0) << 16) / (y1 - y0);
    int32_t invslope2 = ((int32_t)(x2 - x0) << 16) / (y2 - y0);

    // Start positions (16.16 fixed point)
    int32_t curx1 = (int32_t)x0 << 16;
    int32_t curx2 = (int32_t)x0 << 16;

    int16_t screenHeight = display.height();
    int16_t screenWidth = display.width();

    for (int16_t scanlineY = y0; scanlineY <= y1; scanlineY++) {
        // Bounds check Y
        if (scanlineY >= 0 && scanlineY < screenHeight) {
            // Convert from fixed point to integer
            int16_t sx = (int16_t)(curx1 >> 16);
            int16_t ex = (int16_t)(curx2 >> 16);

            // Ensure sx < ex
            if (sx > ex) std::swap(sx, ex);

            // Clip to screen bounds
            if (sx < 0) sx = 0;
            if (ex >= screenWidth) ex = screenWidth - 1;

            // Draw horizontal line
            if (ex >= sx && sx < screenWidth && ex >= 0) {
                display.drawFastHLine(sx, scanlineY, ex - sx + 1,
                                      display::Color(color));
            }
        }

        // Advance X positions
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void fillFlatTopTriangle(display::Display& display, int16_t x0, int16_t y0,
                         int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                         uint16_t color) {
    // v0 and v1 are top vertices (y0 == y1), v2 is bottom
    if (y2 <= y0) return; // Degenerate

    // Calculate inverse slopes using fixed-point
    int32_t invslope1 = ((int32_t)(x2 - x0) << 16) / (y2 - y0);
    int32_t invslope2 = ((int32_t)(x2 - x1) << 16) / (y2 - y1);

    // Start positions (16.16 fixed point)
    int32_t curx1 = (int32_t)x2 << 16;
    int32_t curx2 = (int32_t)x2 << 16;

    int16_t screenHeight = display.height();
    int16_t screenWidth = display.width();

    // Scan from bottom to top
    for (int16_t scanlineY = y2; scanlineY > y0; scanlineY--) {
        // Bounds check Y
        if (scanlineY >= 0 && scanlineY < screenHeight) {
            // Convert from fixed point to integer
            int16_t sx = (int16_t)(curx1 >> 16);
            int16_t ex = (int16_t)(curx2 >> 16);

            // Ensure sx < ex
            if (sx > ex) std::swap(sx, ex);

            // Clip to screen bounds
            if (sx < 0) sx = 0;
            if (ex >= screenWidth) ex = screenWidth - 1;

            // Draw horizontal line
            if (ex >= sx && sx < screenWidth && ex >= 0) {
                display.drawFastHLine(sx, scanlineY, ex - sx + 1,
                                      display::Color(color));
            }
        }

        // Advance X positions
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

} // namespace Raster
