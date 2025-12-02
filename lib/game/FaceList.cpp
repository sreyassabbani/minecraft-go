#include "FaceList.h"

// Undefine Arduino min/max macros to avoid conflicts with std::min/max
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>

FaceList::FaceList() : triCount(0) {
}

bool FaceList::add(const Triangle& tri) {
    if (triCount >= MAX_TRIANGLES) {
        return false; // Buffer full
    }
    
    triangles[triCount++] = tri;
    return true;
}

void FaceList::clear() {
    triCount = 0;
}

void FaceList::sortByDepth() {
    // Sort by depth descending (far to near)
    std::sort(triangles, triangles + triCount, [](const Triangle& a, const Triangle& b) {
        return a.depth > b.depth;
    });
}
