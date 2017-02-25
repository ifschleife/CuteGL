#pragma once

#include <QtGlobal>

#include <memory>


#if defined(NDEBUG)
#define DEBUG_CALL(x)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#else
#define DEBUG_CALL(x)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        x;                                                                                                             \
    } while (0)
#endif


struct Vec3D
{
    Vec3D(float x, float y, float z) : x(x), y(y), z(z) {}

    float x;
    float y;
    float z;

    float length() const;
    void normalize();

    friend Vec3D operator+(const Vec3D& lhs, const Vec3D& rhs);
    friend Vec3D operator/(const Vec3D& lhs, const Vec3D& rhs);
    Vec3D& operator/(float d);
    Vec3D& operator*=(float m);
    Vec3D& operator*(float m);

    bool operator==(const Vec3D& other) const
    {
        return other.x == x && other.y == y && other.z == z;
    }

    bool operator!=(const Vec3D& other) const
    {
        return !(*this == other);
    }
};

float deg_to_rad(float degrees);

auto generate_checker_board_texture(int width, int height) -> std::unique_ptr<class QImage>;
