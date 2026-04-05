#pragma once

#include "math/Vec4.h"
#include "math/Vec2.h"
#include "math/Color4b.h"


namespace rk {
    constexpr int MAX_SPRITES_COUNT = 500;

    constexpr f32 SPRITES_VERTICES[] = {
        1.f, 1.f,  1.f, 1.f, // bottom right
        1.f, 0.f,  1.f, 0.f, // top right
        0.f, 0.f,  0.f, 0.f, // top left
        0.f, 1.f,  0.f, 1.f, // bottom left
    };

    constexpr u32 SPRITES_INDICES[] = { 0, 1, 2, 2, 3, 0 };


    struct SpriteVertices {
        Vec2 position;
        Vec2 size;
        Vec4 uv;
        Color4b color;
        u8 depth;
    };
}