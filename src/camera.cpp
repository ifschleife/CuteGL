#include "camera.h"

#include "util.h"

#include <math.h>


Camera::Camera(QVector3D&& pos)
  : m_pitch{90.0f}
  , m_pos{pos}
  , m_view_dir{0.0f, 0.0f, 0.0f}
  , m_view_matrix{}
  , m_yaw{90.0f}
{
    calculate_view_matrix();
}

const QMatrix4x4& Camera::get_view() const
{
    return m_view_matrix;
}

void Camera::change_pitch(float angle)
{
    m_pitch += angle;
    m_pitch = std::abs(m_pitch);
    m_pitch = std::min(m_pitch, 179.9f);
    m_pitch = std::max(m_pitch, 0.1f);
    calculate_view_matrix();
}

void Camera::change_yaw(float angle)
{
    m_yaw += angle;
    calculate_view_matrix();
}

void Camera::move_backward(float dist)
{
    m_pos -= dist * m_view_dir;
    calculate_view_matrix();
}

void Camera::move_forward(float dist)
{
    m_pos += dist * m_view_dir;
    calculate_view_matrix();
}

void Camera::move_left(float dist)
{
    const float dir = m_yaw + 90.0f;
    const float x = dist * cos(deg_to_rad(dir));
    const float y = dist * sin(deg_to_rad(dir));
    m_pos += {x, y, 0.0f};
    calculate_view_matrix();
}

void Camera::move_right(float dist)
{
    const float dir = m_yaw - 90.0f;
    const float x = dist * cos(deg_to_rad(dir));
    const float y = dist * sin(deg_to_rad(dir));
    m_pos += {x, y, 0.0f};
    calculate_view_matrix();
}

void Camera::calculate_view_dir()
{
    m_view_dir.setX(sin(deg_to_rad(m_pitch)) * cos(deg_to_rad(m_yaw)));
    m_view_dir.setY(sin(deg_to_rad(m_pitch)) * sin(deg_to_rad(m_yaw)));
    m_view_dir.setZ(cos(deg_to_rad(m_pitch)));
}

void Camera::calculate_view_matrix()
{
    calculate_view_dir();

    const QVector3D center = m_pos + m_view_dir;
    const QVector3D dir = (m_pos - center).normalized();

    const QVector3D right = QVector3D::crossProduct(dir, {0.0f, 0.0f, 1.0f});
    const QVector3D up = QVector3D::crossProduct(right, dir);

    m_view_matrix.setToIdentity();
    m_view_matrix.lookAt(m_pos, center, up);
}
