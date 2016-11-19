#pragma once

#include <QMatrix4x4>


class Camera
{
public:
    explicit Camera(QVector3D&& pos);

    QMatrix4x4 get_view() const;
    void move_backward(float dist);
    void move_forward(float dist);
    void move_left(float dist);
    void move_right(float dist);
    void rotate(float angle);

private:
    float     m_angle;
    QVector3D m_pos;
};
