#include "opengl_window.h"

#include <QtMath>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QTimer>

#include <chrono>
#include <stdexcept>

#include "framebuffer.h"
#include "util.h"


namespace
{
	const float ANIMATION_SPEED           = 100.0f;
	const uint_fast16_t RESOLUTION_WIDTH  = 1920;
	const uint_fast16_t RESOLUTION_HEIGHT = 1080;
}


OpenGLWindow::OpenGLWindow()
	: m_framebuffer(std::make_unique<Framebuffer>())
	, m_frame_timer(std::make_unique<QTimer>())
	, m_logger(std::make_unique<QOpenGLDebugLogger>())
	, m_post_process_program(std::make_unique<QOpenGLShaderProgram>())
	, m_program(std::make_unique<QOpenGLShaderProgram>())
{
	m_frame_timer->setInterval(1000);

	DEBUG_CALL(connect(m_logger.get(), &QOpenGLDebugLogger::messageLogged, this, &OpenGLWindow::handle_log_message));
	connect(m_frame_timer.get(), &QTimer::timeout, this, &OpenGLWindow::updateFrameTime);
}

OpenGLWindow::~OpenGLWindow() = default;

void OpenGLWindow::initializeGL()
{
	initializeOpenGLFunctions();

	DEBUG_CALL(m_logger->initialize());
	DEBUG_CALL(m_logger->startLogging(QOpenGLDebugLogger::SynchronousLogging));

	m_framebuffer->initialize(RESOLUTION_WIDTH, RESOLUTION_HEIGHT);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// shaders for shape
	bool shaders_ok = true;
	shaders_ok |= m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../src/shaders/normal_vs.glsl");
	shaders_ok |= m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../src/shaders/normal_fs.glsl");
	shaders_ok |= m_program->link();
	if (!shaders_ok)
	{
		qDebug() << m_program->log();
	}
	
	glGenBuffers(1, &m_vbo_uniform);
	glBindBuffer(GL_UNIFORM_BUFFER, m_vbo_uniform);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_uniform_block), (void*)&m_uniform_block, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	m_posAttr = m_program->attributeLocation("position");
	m_textureCoordAttr = m_program->attributeLocation("textureCoordAttr");
	m_texture_uniform = m_program->uniformLocation("tex");

	// shaders for framebuffer quad
	shaders_ok = true;
	shaders_ok |= m_post_process_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../src/shaders/post_process_vs.glsl");
	shaders_ok |= m_post_process_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../src/shaders/post_process_fs.glsl");
	shaders_ok |= m_post_process_program->link();
	if (!shaders_ok)
	{
		qDebug() << m_program->log();
	}
	//char buffer[512];
	//glGetShaderInfoLog(m_program->shaders()[0]->shaderId(), 512, nullptr, buffer);
	//qDebug() << buffer;

	m_post_process_pos_attr = m_post_process_program->attributeLocation("position");
	m_post_process_tex_uniform = m_post_process_program->uniformLocation("tex");

	// vbo for a quad
	GLfloat quad[] =
	{
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};
	glGenBuffers(1, &m_vbo_quad);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	add_sub_div_sphere({0.0f, 0.0f, 0.5f}, 0.5f);

	// vbo for triangle
	glGenBuffers(1, &m_shape.vbos.pos);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbos.pos);
	glBufferData(GL_ARRAY_BUFFER, m_shape.positions.size() * sizeof(m_shape.positions[0]), &m_shape.positions[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_shape.vbos.index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shape.vbos.index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_shape.indices.size() * sizeof(m_shape.indices[0]), &m_shape.indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLfloat texture_coords[] =
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.5f, 0.5f
	};
	glGenBuffers(1, &m_vbo_texture_coords);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_texture_coords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLfloat texture_data[] =
	{
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	glGenTextures(1, &m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2, 2, 0, GL_RGBA, GL_FLOAT, texture_data);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_frame_timer->start();
}


void OpenGLWindow::paintGL()
{
	static auto start = std::chrono::high_resolution_clock::now();
	const auto now = std::chrono::high_resolution_clock::now();
	const float time = std::chrono::duration_cast<std::chrono::duration<float>>(now - start).count();

	const qreal retinaScale = devicePixelRatio();
	glViewport(0, 0, static_cast<GLsizei>(width() * retinaScale), static_cast<GLsizei>(height() * retinaScale));

	if (m_animating)
	{
		m_angle = fmod(ANIMATION_SPEED * time, 360.0f);
	}

	QMatrix4x4 model;
	model.rotate(m_angle, {0.0f, 1.0f, 0.0f});

	QMatrix4x4 view;
	view.lookAt
	(
		{1.0f, 1.0f, 1.0f}, // world space
		{0.0f, 0.0f, 0.0f}, // world space
		{0.0f, 0.0f, 1.0f}
	);

	QMatrix4x4 proj;
	proj.perspective(60.0f, width() / (float)height(), 0.1f, 100.0f);

	const QMatrix4x4 matrix = proj * view * model;
	m_uniform_block.matrix = matrix;

	m_program->bind();

	m_program->setUniformValue(m_texture_uniform, 0); // 0 == texture slot number from glActiveTexture

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_vbo_uniform);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_uniform_block), (void*)&m_uniform_block);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glEnableVertexAttribArray(0); // enable attribute slot for shader input variable
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbos.pos);
	glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_vertices
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// re-direct rendering to frame buffer
	m_framebuffer->clear();

	GLfloat texture_data[] =
	{
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	texture_data[1] = (sin(time*4.0f) + 1.0f) / 2.0f;
	texture_data[4] = (sin(time*4.0f) + 1.0f) / 2.0f;
	texture_data[9] = (sin(time*4.0f) + 1.0f) / 2.0f;
	glTextureSubImage2D(m_texture_id, 0, 0, 0, 2, 2, GL_RGBA, GL_FLOAT, texture_data);
	
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT_FACE);

	// actual draw call of the shape (triangle)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shape.vbos.index);
	if (m_show_wire_frame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_shape.indices.size(), GL_UNSIGNED_INT, nullptr);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_program->release();
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	// switch back to back buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST); // fb does not have depth
	glDisable(GL_CULL_FACE);

	m_post_process_program->bind();
	m_post_process_program->setUniformValue(m_post_process_tex_uniform, 0); // 0 == texture slot number from glActiveTexture

	m_framebuffer->bind_color_texture();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
	glVertexAttribPointer(m_post_process_pos_attr, 2, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	m_post_process_program->release();

	// clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0); // make sure we unbind the correct texture slot
	glBindTexture(GL_TEXTURE_2D, 0);
	
	++m_frame_counter;
}

void OpenGLWindow::setAnimating(bool animating)
{
	m_animating = animating;
    if (animating)
    {
        // Animate continuously, throttled by the blocking swapBuffers() call the
        // QOpenGLWindow internally executes after each paint. Once that is done
        // (frameSwapped signal is emitted), we schedule a new update. This
        // obviously assumes that the swap interval (see
        // QSurfaceFormat::setSwapInterval()) is non-zero.
        connect(this, &OpenGLWindow::frameSwapped, this, qOverload<>(&OpenGLWindow::update));
        
		update();
    }
    else
    {
        disconnect(this, &OpenGLWindow::frameSwapped, this, qOverload<>(&OpenGLWindow::update));
    }
}

void OpenGLWindow::showWireFrame(bool status)
{
	m_show_wire_frame = status;
	requestUpdate();
}

void OpenGLWindow::updateFrameTime()
{
	emit frameTime(m_frame_counter / 3.60f);
	m_frame_counter = 0;
}


void OpenGLWindow::handle_log_message(const QOpenGLDebugMessage& msg)
{
	qDebug() << msg;
	if (msg.severity() == QOpenGLDebugMessage::HighSeverity)
	{
		throw std::runtime_error(msg.message().toStdString());
	}
}

void OpenGLWindow::add_sub_div_sphere(const Vec3D& pos, float size)
{
	const float t = (1.0f + sqrt(5.0f)) / 4.0f;

	m_shape.add_normalized_vertex({-0.5f,  t, 0.0f});
	m_shape.add_normalized_vertex({0.5f,  t, 0.0f});
	m_shape.add_normalized_vertex({-0.5f, -t, 0.0f});
	m_shape.add_normalized_vertex({0.5f, -t, 0.0f});

	m_shape.add_normalized_vertex({0.0f, -0.5f,  t});
	m_shape.add_normalized_vertex({0.0f,  0.5f,  t});
	m_shape.add_normalized_vertex({0.0f, -0.5f, -t});
	m_shape.add_normalized_vertex({0.0f,  0.5f, -t});

	m_shape.add_normalized_vertex({t, 0.0f, -0.5f});
	m_shape.add_normalized_vertex({t, 0.0f,  0.5f});
	m_shape.add_normalized_vertex({-t, 0.0f, -0.5f});
	m_shape.add_normalized_vertex({-t, 0.0f,  0.5f});

	m_shape.indices.push_back({0, 11,  5});
	m_shape.indices.push_back({0,  5,  1});
	m_shape.indices.push_back({0,  1,  7});
	m_shape.indices.push_back({0,  7, 10});
	m_shape.indices.push_back({0, 10, 11});

	m_shape.indices.push_back({1,  5,  9});
	m_shape.indices.push_back({5, 11,  4});
	m_shape.indices.push_back({11, 10,  2});
	m_shape.indices.push_back({10,  7,  6});
	m_shape.indices.push_back({7,  1,  8});

	m_shape.indices.push_back({3,  9,  4});
	m_shape.indices.push_back({3,  4,  2});
	m_shape.indices.push_back({3,  2,  6});
	m_shape.indices.push_back({3,  6,  8});
	m_shape.indices.push_back({3,  8,  9});

	m_shape.indices.push_back({4,  9,  5});
	m_shape.indices.push_back({2,  4, 11});
	m_shape.indices.push_back({6,  2, 10});
	m_shape.indices.push_back({8,  6,  7});
	m_shape.indices.push_back({9,  8,  1});

	m_shape.sub_divide(4);
	m_shape.scale(size);
}

void OpenGLWindow::resizeGL(int width, int height)
{
	m_framebuffer->resize(width, height);
}
