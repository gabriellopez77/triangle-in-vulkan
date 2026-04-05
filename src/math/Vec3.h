#pragma once

#include "Defs.h"


class Vec3 {
public:
    constexpr Vec3() {
        x = 0.f;
        y = 0.f;
        z = 0.f;
    }

    constexpr Vec3(f32 value) {
        x = value;
        y = value;
        z = value;
    }

    constexpr Vec3(f32 x, f32 y, f32 z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    f32 x;
    f32 y;
    f32 z;


    Vec3 operator+(const Vec3& r) const { return { x + r.x, y + r.y, z + r.z }; }
    Vec3 operator-(const Vec3& r) const { return { x - r.x, y - r.y, z - r.z }; }
    Vec3 operator*(const Vec3& r) const { return { x * r.x, y * r.y, z * r.z }; }
    Vec3 operator/(const Vec3& r) const { return { x / r.x, y / r.y, z / r.z }; }

    Vec3 operator+=(const Vec3& r) { return { x += r.x, y += r.y, z += r.z }; }
    Vec3 operator-=(const Vec3& r) { return { x -= r.x, y -= r.y, z -= r.z }; }
    Vec3 operator*=(const Vec3& r) { return { x *= r.x, y *= r.y, z *= r.z }; }
    Vec3 operator/=(const Vec3& r) { return { x /= r.x, y /= r.y, z /= r.z }; }

    Vec3 operator+(f32 value) const { return { x + value, y + value, z + value }; }
    Vec3 operator-(f32 value) const { return { x - value, y - value, z - value }; }
    Vec3 operator*(f32 value) const { return { x * value, y * value, z * value }; }
    Vec3 operator/(f32 value) const { return { x / value, y / value, z / value }; }

    Vec3 operator+=(f32 value) { return { x += value, y += value, z += value }; }
    Vec3 operator-=(f32 value) { return { x -= value, y -= value, z -= value }; }
    Vec3 operator*=(f32 value) { return { x *= value, y *= value, z *= value }; }
    Vec3 operator/=(f32 value) { return { x /= value, y /= value, z /= value }; }

    bool operator==(const Vec3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vec3& other) const { return !(*this == other); }


    static Vec3 cross(const Vec3& left, const Vec3& right);
    static Vec3 lerp(const Vec3& a, const Vec3& b, f32 t);
    static Vec3 normalize(const Vec3& vec);
    static f32 dot(const Vec3& left, const Vec3& right);

    void normalize();
    Vec3 normalized() const;
    f32 length() const;
};