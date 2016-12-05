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
    const float x = sin(deg_to_rad(m_pitch)) * cos(deg_to_rad(m_yaw));
    const float y = sin(deg_to_rad(m_pitch)) * sin(deg_to_rad(m_yaw));
    const float z = cos(deg_to_rad(m_pitch));
    const QVector3D tgt = m_pos + QVector3D{x, y, z};

    QMatrix4x4 view;
    view.lookAt(m_pos, tgt, {0.0f, 0.0f, 1.0f});
    return view;
}

void Camera::change_pitch(float angle)
{
    m_pitch += angle;
    m_pitch = std::abs(m_pitch);
    m_pitch = std::min(m_pitch, 179.9f);
    m_pitch = std::max(m_pitch, 0.1f);
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
