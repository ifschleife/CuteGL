#include "camera.h"

#include "util.h"

#include <math.h>


Camera::Camera(QVector3D&& pos)
    : m_angle(90.0f)
    , m_pos(pos)
{
}

QMatrix4x4 Camera::get_view() const
{
    QMatrix4x4 view;
    QVector3D tgt = m_pos;
    const float cs = cos(deg_to_rad(m_angle));
    const float si = sin(deg_to_rad(m_angle));
    tgt += {cs, si, 0.0f};
    view.lookAt(m_pos, tgt, {0.0f, 0.0f, 1.0f});
    return view;
}

void Camera::move_backward(float dist)
{
    const float x = dist * cos(deg_to_rad(m_angle));
    const float y = dist * sin(deg_to_rad(m_angle));
    m_pos -= {x, y, 0.0f};
}

void Camera::move_forward(float dist)
{
    const float x = dist * cos(deg_to_rad(m_angle));
    const float y = dist * sin(deg_to_rad(m_angle));
    m_pos += {x, y, 0.0f};
}

void Camera::move_left(float dist)
{
    const float dir = m_angle + 90.0f;
    const float x = dist * cos(deg_to_rad(dir));
    const float y = dist * sin(deg_to_rad(dir));
    m_pos += {x, y, 0.0f};
}

void Camera::move_right(float dist)
{
    const float dir = m_angle - 90.0f;
    const float x = dist * cos(deg_to_rad(dir));
    const float y = dist * sin(deg_to_rad(dir));
    m_pos += {x, y, 0.0f};
}

void Camera::rotate(float angle)
{
    m_angle += angle;
}
