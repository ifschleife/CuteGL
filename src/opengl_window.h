#pragma once

#include <QMatrix4x4>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLWindow>

#include <chrono>
#include <memory>

#include "shape.h"


class Framebuffer;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLShaderProgram;
class QTimer;
class Shader;

struct UniformBlock
{
	QMatrix4x4 matrix;
};


class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_3_Core
{
	Q_OBJECT

public:
	OpenGLWindow();
    ~OpenGLWindow() override;

	void initializeGL() override;
	void paintGL() override;

signals:
	void frameTime(float time_in_ms);

public slots:
	void setAnimating(bool animating);
	void showWireFrame(bool status);
	void updateFrameTime();

private slots:
	void handle_log_message(const QOpenGLDebugMessage& msg);

private:
	void add_sub_div_sphere(const Vec3D& pos, float size);
	void resizeGL(int width, int height) override;

private:
    std::unique_ptr<Framebuffer> m_framebuffer;

    std::unique_ptr<QTimer> m_frame_timer;
    uint_fast8_t m_frame_counter{0};

    std::unique_ptr<QOpenGLDebugLogger> m_logger;

    std::unique_ptr<Shader> m_post_process_shader;

	GLuint m_vbo_quad;

    bool m_animating{false};
	float m_angle{0.0f};

	bool m_show_wire_frame{false};

	Shape m_sphere;
	std::unique_ptr<Shader> m_sphere_shader;
	UniformBlock m_sphere_ub;

	Shape m_plane;
	std::unique_ptr<Shader> m_plane_shader;
	UniformBlock m_plane_ub;
};
