#pragma once

#include "Defs.h"


class Vec4 {
public:
    constexpr Vec4() {
        x = 0.f;
        y = 0.f;
        z = 0.f;
        w = 0.f;
    }

    constexpr Vec4(f32 value) {
        x = value;
        y = value;
        z = value;
        w = value;
    }

    constexpr Vec4(f32 x, f32 y, f32 z, f32 w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    f32 x;
    f32 y;
    f32 z;
    f32 w;


    Vec4 operator+(const Vec4& r) const { return { x + r.x, y + r.y, z + r.z, w + r.w }; }
    Vec4 operator-(const Vec4& r) const { return { x - r.x, y - r.y, z - r.z, w - r.w }; }
    Vec4 operator*(const Vec4& r) const { return { x * r.x, y * r.y, z * r.z, w * r.w }; }
    Vec4 operator/(const Vec4& r) const { return { x / r.x, y / r.y, z / r.z, w / r.w }; }

    Vec4 operator+=(const Vec4& r) { return { x += r.x, y += r.y, z += r.z, w += r.w }; }
    Vec4 operator-=(const Vec4& r) { return { x -= r.x, y -= r.y, z -= r.z, w -= r.w }; }
    Vec4 operator*=(const Vec4& r) { return { x *= r.x, y *= r.y, z *= r.z, w *= r.w }; }
    Vec4 operator/=(const Vec4& r) { return { x /= r.x, y /= r.y, z /= r.z, w /= r.w }; }

    Vec4 operator+(f32 value) const { return { x + value, y + value, z + value, w + value }; }
    Vec4 operator-(f32 value) const { return { x - value, y - value, z - value, w - value }; }
    Vec4 operator*(f32 value) const { return { x * value, y * value, z * value, w * value }; }
    Vec4 operator/(f32 value) const { return { x / value, y / value, z / value, w / value }; }

    Vec4 operator+=(f32 value) { return { x += value, y += value, z += value, w += value }; }
    Vec4 operator-=(f32 value) { return { x -= value, y -= value, z -= value, w -= value }; }
    Vec4 operator*=(f32 value) { return { x *= value, y *= value, z *= value, w *= value }; }
    Vec4 operator/=(f32 value) { return { x /= value, y /= value, z /= value, w /= value }; }

    bool operator==(const Vec4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const Vec4& other) const { return !(*this == other); }


    static Vec4 lerp(const Vec4& a, const Vec4& b, f32 t);
    static Vec4 normalize(const Vec4& vec);
    static f32 dot(const Vec4& left, const Vec4& right);

    void normalize();
    Vec4 normalized() const;
    f32 length() const;
};