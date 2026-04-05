#pragma once

#include "Defs.h"


class Vec2 {
public:
    constexpr Vec2() {
        x = 0.f;
        y = 0.f;
    }

    constexpr Vec2(f32 value) {
        x = value;
        y = value;
    }

    constexpr Vec2(f32 x, f32 y) {
        this->x = x;
        this->y = y;
    }

    f32 x;
    f32 y;


    Vec2 operator+(const Vec2& r) const { return { x + r.x, y + r.y }; }
    Vec2 operator-(const Vec2& r) const { return { x - r.x, y - r.y }; }
    Vec2 operator*(const Vec2& r) const { return { x * r.x, y * r.y }; }
    Vec2 operator/(const Vec2& r) const { return { x / r.x, y / r.y }; }

    Vec2 operator+=(const Vec2& r) { return { x += r.x, y += r.y }; }
    Vec2 operator-=(const Vec2& r) { return { x -= r.x, y -= r.y }; }
    Vec2 operator*=(const Vec2& r) { return { x *= r.x, y *= r.y }; }
    Vec2 operator/=(const Vec2& r) { return { x /= r.x, y /= r.y }; }

    Vec2 operator+(f32 value) const { return { x + value, y + value }; }
    Vec2 operator-(f32 value) const { return { x - value, y - value }; }
    Vec2 operator*(f32 value) const { return { x * value, y * value }; }
    Vec2 operator/(f32 value) const { return { x / value, y / value }; }

    Vec2 operator+=(f32 value) { return { x += value, y += value, }; }
    Vec2 operator-=(f32 value) { return { x -= value, y -= value, }; }
    Vec2 operator*=(f32 value) { return { x *= value, y *= value, }; }
    Vec2 operator/=(f32 value) { return { x /= value, y /= value, }; }

    bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2& other) const { return !(*this == other); }


    static Vec2 lerp(const Vec2& a, const Vec2& b, f32 t);
    static Vec2 normalize(const Vec2& vec);
    static f32 dot(const Vec2& left, const Vec2& right);

    void normalize();
    Vec2 normalized() const;
    f32 length() const;
};