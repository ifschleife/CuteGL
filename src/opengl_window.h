#pragma once

#include <QMatrix4x4>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWindow>

#include <chrono>
#include <memory>

#include "shape.h"


class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLShaderProgram;
class QTimer;

struct UniformBlock
{
	QMatrix4x4 matrix;
};

class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
	Q_OBJECT

public:
	OpenGLWindow();

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
	GLuint loadShader(GLenum type, const char* source);

private:
	std::unique_ptr<QOpenGLDebugLogger> m_logger;

	std::unique_ptr<QOpenGLShaderProgram> m_post_process_program;
	std::unique_ptr<QOpenGLShaderProgram> m_program;

	GLuint m_posAttr;
	GLuint m_textureCoordAttr;
	
	GLuint m_index_buffer;
	GLuint m_vbo_texture_coords;
	GLuint m_texture_id;
	GLuint m_texture_uniform;

	GLuint m_fb_id;
	GLuint m_fb_col_id;
	GLuint m_fb_depth_id;

	GLuint m_post_process_pos_attr;
	GLuint m_post_process_tex_uniform;

	GLuint m_vbo_quad;

	UniformBlock m_uniform_block;
	GLuint m_vbo_uniform;

	std::unique_ptr<QTimer> m_frame_timer;
	uint_fast8_t m_frame_counter{0};

	bool m_animating{false};
	float m_angle{0.0f};

	bool m_show_wire_frame{false};

	Shape m_shape;
};
