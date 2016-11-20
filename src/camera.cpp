#include "camera.h"

#include "util.h"

#include <math.h>


Camera::Camera(QVector3D&& pos)
    : m_pitch{90.0f}
    , m_pos(pos)
    , m_yaw(90.0f)
{
}

QMatrix4x4 Camera::get_view() const
{
    QMatrix4x4 view;
    QVector3D tgt = m_pos;
    const float x = cos(deg_to_rad(m_yaw));
    const float y = sin(deg_to_rad(m_yaw));
    const float z = cos(deg_to_rad(m_pitch));
    tgt += {x, y, z};
    view.lookAt(m_pos, tgt, {0.0f, 0.0f, 1.0f});
    return view;
}

void Camera::change_pitch(float angle)
{
    m_pitch += angle;
}

void Camera::change_yaw(float angle)
{
    m_yaw += angle;
}

void Camera::move_backward(float dist)
{
    const float x = dist * cos(deg_to_rad(m_yaw));
    const float y = dist * sin(deg_to_rad(m_yaw));
    const float z = dist * cos(deg_to_rad(m_pitch));
    m_pos -= {x, y, z};
}

void Camera::move_forward(float dist)
{
    const float x = dist * cos(deg_to_rad(m_yaw));
    const float y = dist * sin(deg_to_rad(m_yaw));
    const float z = dist * cos(deg_to_rad(m_pitch));
    m_pos += {x, y, z};
}

void Camera::move_left(float dist)
{
    const float dir = m_yaw + 90.0f;
    const float x = dist * cos(deg_to_rad(dir));
    const float y = dist * sin(deg_to_rad(dir));
    m_pos += {x, y, 0.0f};
}

void Camera::move_right(float dist)
{
    const float dir = m_yaw - 90.0f;
    const float x = dist * cos(deg_to_rad(dir));
    const float y = dist * sin(deg_to_rad(dir));
    m_pos += {x, y, 0.0f};
}
