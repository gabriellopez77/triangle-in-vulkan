#pragma once

#include "Color3b.h"


struct Color4b
{
    Color4b(Color3b color, u8 a)
    {
        r = color.r;
        g = color.g;
        b = color.b;
        this->a = a;
    }
    
    Color4b(u8 r, u8 g, u8 b, u8 a)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }
    
    Color4b(u8 color)
    { 
        r = color; 
        g = color; 
        b = color;
        a = color;
    }
    
    u8 r;
    u8 g;
    u8 b;
    u8 a;

    static Color4b FromHex(u32 hex) {
        u8 r = (u8)(hex >> 24);
        u8 g = (u8)(hex >> 16);
        u8 b = (u8)(hex >> 8);
        u8 a = (u8)(hex >> 0);
        return {r, g, b, a };
    }
};