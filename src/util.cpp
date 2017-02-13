#include "util.h"

#include <cassert>
#include <math.h>
#include <QImage>


float Vec3D::length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

void Vec3D::normalize()
{
    *this = *this / length();
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

auto generate_checker_board_texture(int width, int height) -> std::unique_ptr<QImage>
{
    auto img = std::make_unique<QImage>(width, height, QImage::Format_RGB32);

    bool white = true;
    for (int y = 0; y < height; ++y)
    {
        uchar* line = img->scanLine(y);
        for (int x = 0; x < width * 4; x += 4)
        {
            line[x] = white ? 255 : 0;
            line[x + 1] = white ? 255 : 0;
            line[x + 2] = white ? 255 : 0;
            line[x + 3] = 255; // alpha
            white = !white;
        }
        white = !white; // for next line
    }

    return img;
}
