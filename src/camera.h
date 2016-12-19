#pragma once

#include <QMatrix4x4>


class Camera
{
  public:
    explicit Camera(QVector3D&& pos);

    void change_pitch(float angle);
    void change_yaw(float angle);

    QMatrix4x4 get_view() const;

    void move_backward(float dist);
    void move_forward(float dist);
    void move_left(float dist);
    void move_right(float dist);

  private:
    float m_pitch;
    QVector3D m_pos;
    float m_yaw;
};
