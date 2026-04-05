#pragma once

#include "Defs.h"


struct Color3b
{
    Color3b(u8 r, u8 g, u8 b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    Color3b(u8 color)
    {
        r = color;
        g = color;
        b = color;
    }

    u8 r;
    u8 g;
    u8 b;

    static Color3b FromHex(u32 hex)  {
        u8 r = (u8)(hex >> 16);
        u8 g = (u8)(hex >> 8);
        u8 b = (u8)(hex >> 0);

        return { r, g, b };
    }
};