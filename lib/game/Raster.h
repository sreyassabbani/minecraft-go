#ifndef RASTER_H
#define RASTER_H

#include "../../lib/display/display.hpp"
#include <stdint.h>

// Fast triangle rasterizer based on GBA techniques
// Uses integer math and scanline filling for performance
namespace Raster {
    
    // Fill a triangle using scanline algorithm
    // Vertices should be in screen space (pixel coordinates)
    // Based on Ben Carter's GBA rasterizer approach
    void fillTriangle(
        display::Display& display,
        int16_t x0, int16_t y0,
        int16_t x1, int16_t y1,
        int16_t x2, int16_t y2,
        uint16_t color
    );
    
    // Fill a flat-bottomed triangle (v0 at top, v1 and v2 at bottom with same y)
    void fillFlatBottomTriangle(
        display::Display& display,
        int16_t x0, int16_t y0,
        int16_t x1, int16_t y1,
        int16_t x2, int16_t y2,
        uint16_t color
    );
    
    // Fill a flat-topped triangle (v0 and v1 at top with same y, v2 at bottom)
    void fillFlatTopTriangle(
        display::Display& display,
        int16_t x0, int16_t y0,
        int16_t x1, int16_t y1,
        int16_t x2, int16_t y2,
        uint16_t color
    );
    
} // namespace Raster

#endif
