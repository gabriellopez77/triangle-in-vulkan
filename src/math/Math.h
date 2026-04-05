#pragma once

#include "Defs.h"


namespace math {
    constexpr inline f32 radians(f32 degrees) { return degrees * 0.01745329f; }

    constexpr inline f32 min(f32 a, f32 b) { return a < b ? a : b; }
    constexpr inline i32 min(i32 a, i32 b) { return a < b ? a : b; }
    constexpr inline u32 min(u32 a, u32 b) { return a < b ? a : b; }

    constexpr inline f32 max(f32 a, f32 b) { return a > b ? a : b; }
    constexpr inline i32 max(i32 a, i32 b) { return a > b ? a : b; }
    constexpr inline u32 max(u32 a, u32 b) { return a > b ? a : b; }

    constexpr inline f32 clamp(f32 value, f32 minV, f32 maxV) { return min(max(value, minV), maxV); }
    constexpr inline i32 clamp(i32 value, i32 minV, i32 maxV) { return min(max(value, minV), maxV); }
    constexpr inline u32 clamp(u32 value, u32 minV, u32 maxV) { return min(max(value, minV), maxV); }
    
    extern float cos(f32 value);
    extern float sin(f32 value);
    extern float tan(f32 value);

    // aligns the value to alignment
    constexpr inline i32 alignUp(i32 value, i32 alignment) { return ((value + alignment - 1) / alignment) * value; };
    constexpr inline u32 alignUp(u32 value, u32 alignment) { return ((value + alignment - 1) / alignment) * value; };

    constexpr inline bool isAligned(i32 value, i32 alignment) { return (value % alignment) == 0; }
    constexpr inline bool isAligned(u32 value, u32 alignment) { return (value % alignment) == 0; }
    constexpr inline bool isAligned(u64 value, u64 alignment) { return (value % alignment) == 0; }

    constexpr inline f32 lerp(f32 a, f32 b, f32 t) { return a + (b - a) * t; }
}
