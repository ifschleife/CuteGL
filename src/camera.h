#pragma once

#include <QMatrix4x4>


class Camera
{
public:
    explicit Camera(QVector3D&& pos);

    void change_pitch(float angle);
    void change_yaw(float angle);

    struct Vec3D getPosition() const;
    const QMatrix4x4& get_view() const;
    struct Vec3D getViewDirection() const;

    void move_backward(float dist);
    void move_forward(float dist);
    void move_left(float dist);
    void move_right(float dist);

private:
    void calculate_view_dir();
    void calculate_view_matrix();

private:
    float m_pitch;
    QVector3D m_pos;
    QVector3D m_view_dir;
    QMatrix4x4 m_view_matrix;
    float m_yaw;
};
