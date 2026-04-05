#include "Matrix4.h"

#include "Math.h"
#include "Vec3.h"


Matrix4 Matrix4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    const Vec3 f(Vec3::normalize(target - eye));
    const Vec3 s(Vec3::normalize(Vec3::cross(f, up)));
    const Vec3 u(Vec3::cross(s, f));

    Matrix4 result(1.f);
    result[0].x =  s.x;
    result[1].x =  s.y;
    result[2].x =  s.z;
    result[0].y =  u.x;
    result[1].y =  u.y;
    result[2].y =  u.z;
    result[0].z = -f.x;
    result[1].z = -f.y;
    result[2].z = -f.z;
    result[3].x = -Vec3::dot(s, eye);
    result[3].y = -Vec3::dot(u, eye);
    result[3].z =  Vec3::dot(f, eye);

    return result;
}
Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float near, float far) {
    Matrix4 result(0.f);

    float invRL = 1.0f / (right - left);
    float invTB = 1.0f / (top - bottom);
    float invFN = 1.0f / (far - near);

    result[0].x = 2.f * invRL;
    result[1].y = 2.f * invTB;
    result[2].z = -2.f * invFN;

    result[3].x = -(right + left) * invRL;
    result[3].y = -(top + bottom) * invTB;
    result[3].z = -(far + near) * invFN;

    return result;
}

Matrix4 Matrix4::perspective(float fov, float aspect, float near, float far) {
    const float tanHalfFov = math::tan(fov / 2.f);

    Matrix4 result(0.f);
    result[0].x = 1.f / (aspect * tanHalfFov);
    result[1].y = 1.f / tanHalfFov;
    result[2].z = - (far + near) / (far - near);
    result[2].w = - 1.f;
    result[3].z = - (2.f * far * near) / (far - near);

    return result;
}

void Matrix4::translate(const Vec3& translation) {
    Matrix4 result(*this);

    result[3] = values[0] * translation.x + values[1] * translation.y + values[2] * translation.z + values[3];

    *this = result;
}

void Matrix4::scale(const Vec3& scale) {
    Matrix4 result(0.f);
    result[0] = values[0] * scale.x;
    result[1] = values[1] * scale.y;
    result[2] = values[2] * scale.z;
    result[3] = values[3];

    *this = result;
}

void Matrix4::rotate(float angle, const Vec3& v) {
    const float a = math::radians(angle);
    const float c = math::cos(a);
    const float s = math::sin(a);

    const Vec3 axis = Vec3::normalize(v);
    const Vec3 temp(axis * (1 - c));

    Matrix4 rotate(0.f);
    rotate[0].x = c + temp.x * axis.x;
    rotate[0].y = temp.x * axis.y + s * axis.z;
    rotate[0].z = temp.x * axis.z - s * axis.y;

    rotate[1].x = temp.y * axis.x - s * axis.z;
    rotate[1].y = c + temp.y * axis.y;
    rotate[1].z = temp.y * axis.z + s * axis.x;

    rotate[2].x = temp.z * axis.x + s * axis.y;
    rotate[2].y = temp.z * axis.y - s * axis.x;
    rotate[2].z = c + temp.z * axis.z;

    Matrix4 result(0.f);
    result[0] = values[0] * rotate[0].x + values[1] * rotate[0].y + values[2] * rotate[0].z;
    result[1] = values[0] * rotate[1].x + values[1] * rotate[1].y + values[2] * rotate[1].z;
    result[2] = values[0] * rotate[2].x + values[1] * rotate[2].y + values[2] * rotate[2].z;
    result[3] = values[3];

    *this = result;
}