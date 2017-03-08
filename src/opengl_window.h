#pragma once

#include <QMatrix4x4>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWindow>

#include <chrono>
#include <memory>

#include "camera.h"
#include "shape.h"


class Framebuffer;
class RenderObject;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLShaderProgram;
class QTimer;
class Shader;


class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    OpenGLWindow();
    ~OpenGLWindow() override;

    void initializeGL() override;
    void paintGL() override;

    void loadObject(const QString& obj_file);

signals:
    void frameTime(float time_in_ms);

public slots:
    void setAnimating(bool animating);
    void showWireFrame(bool status);
    void updateFrameTime();

private slots:
    void handle_log_message(const QOpenGLDebugMessage& msg);

private:
    void resizeGL(int width, int height) override;

public:
    Camera m_camera;

private:
    std::unique_ptr<Framebuffer> m_framebuffer;

    std::unique_ptr<QTimer> m_frame_timer;
    uint_fast8_t m_frame_counter{0};

    std::unique_ptr<QOpenGLDebugLogger> m_logger;

    std::unique_ptr<Shader> m_post_process_shader;
    GLuint m_vbo_quad;

    bool m_animating{false};

    std::vector<std::unique_ptr<RenderObject>> m_objects;
};
