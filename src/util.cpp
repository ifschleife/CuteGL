#include "util.h"

#include <cassert>
#include <math.h>


float Vec3D::length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

void Vec3D::normalize()
{
    *this = *this / length();
}

Vec3D Vec3D::normalized() const
{
    Vec3D copy(*this);
    copy.normalize();
    return copy;
}

Vec3D operator+(const Vec3D& lhs, const Vec3D& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3D operator/(const Vec3D& lhs, const Vec3D& rhs)
{
    return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
}

Vec3D& Vec3D::operator/(float d)
{
    assert(d != 0.0f);

    this->x /= d;
    this->y /= d;
    this->z /= d;

    return *this;
}

Vec3D& Vec3D::operator*=(float m)
{
    this->x *= m;
    this->y *= m;
    this->z *= m;

    return *this;
}

Vec3D& Vec3D::operator*(float m)
{
    return *this *= m;
}


float deg_to_rad(float degrees) { return degrees * 4.0f * atanf(1.0f) / 180.0f; }
