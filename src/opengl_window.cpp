#include "opengl_window.h"

#include <QtMath>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QTimer>

#include <chrono>
#include <math.h>
#include <stdexcept>

#include "framebuffer.h"
#include "shader.h"
#include "util.h"


namespace
{
	const float ANIMATION_SPEED           = 100.0f;
	const uint_fast16_t RESOLUTION_WIDTH  = 1920;
	const uint_fast16_t RESOLUTION_HEIGHT = 1080;
}


OpenGLWindow::OpenGLWindow()
    : m_camera{{1.0f, 1.0f, 0.5f}}
    , m_framebuffer(std::make_unique<Framebuffer>())
	, m_frame_timer(std::make_unique<QTimer>())
	, m_logger(std::make_unique<QOpenGLDebugLogger>())
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
	m_sphere_shader = std::make_unique<Shader>();
	m_sphere_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../src/shaders/normal_vs.glsl");
	m_sphere_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../src/shaders/normal_fs.glsl");
	if (!m_sphere_shader->link())
	{
		qDebug() << m_sphere_shader->log();
	}
	
	m_sphere_shader->create_uniform_block((void*)&m_sphere_ub, sizeof(m_sphere_ub));

	m_sphere_shader->attributeLocation("position");

	// shaders for framebuffer quad
	m_post_process_shader = std::make_unique<Shader>();
	m_post_process_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../src/shaders/post_process_vs.glsl");
	m_post_process_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../src/shaders/post_process_fs.glsl");
	if (!m_post_process_shader->link())
	{
		qDebug() << m_post_process_shader->log();
	}
	//char buffer[512];
	//glGetShaderInfoLog(m_program->shaders()[0]->shaderId(), 512, nullptr, buffer);
	//qDebug() << buffer;

	m_post_process_shader->attributeLocation("position");
	m_post_process_shader->uniformLocation("tex");

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

	// vbo for sphere
	glGenBuffers(1, &m_sphere.vbos.pos);
	glBindBuffer(GL_ARRAY_BUFFER, m_sphere.vbos.pos);
    glBufferData(GL_ARRAY_BUFFER, m_sphere.positions.size() * sizeof(m_sphere.positions[0]), m_sphere.positions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_sphere.vbos.index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere.vbos.index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_sphere.indices.size() * sizeof(m_sphere.indices[0]), m_sphere.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

/// create plane

	m_plane_shader = std::make_unique<Shader>();

	m_plane_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../src/shaders/plane_vs.glsl");
	m_plane_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../src/shaders/texture_fs.glsl");
	if (!m_plane_shader->link())
	{
		qDebug() << m_plane_shader->log();
	}

	m_plane_shader->create_uniform_block((void*)&m_plane_ub, sizeof(m_plane_ub));

	m_plane.positions.push_back({-8.0f, 0.0f, -8.0f});
	m_plane.positions.push_back({ 8.0f, 0.0f, -8.0f});
	m_plane.positions.push_back({ 8.0f, 0.0f,  8.0f});
	m_plane.positions.push_back({-8.0f, 0.0f,  8.0f});

	m_plane.indices.push_back({0, 1, 2});
	m_plane.indices.push_back({2, 3, 0});

	// vbo for plane
	glGenBuffers(1, &m_plane.vbos.pos);
	glBindBuffer(GL_ARRAY_BUFFER, m_plane.vbos.pos);
	glBufferData(GL_ARRAY_BUFFER, m_plane.positions.size() * sizeof(m_plane.positions[0]), m_plane.positions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_plane.vbos.index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_plane.vbos.index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_plane.indices.size() * sizeof(m_plane.indices[0]), m_plane.indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1); // for byte textures
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const int w = 16;
    const int h = 16;
    const auto tex = generate_checker_board_texture(w, h);

    glGenTextures(1, &m_plane.vbos.texture);
	glBindTexture(GL_TEXTURE_2D, m_plane.vbos.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->rgbSwapped().bits());
    glGenerateMipmap(GL_TEXTURE_2D);
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
        m_angle = std::fmod(ANIMATION_SPEED * time, 360.0f);
	}

    QMatrix4x4 model;
    model.translate(0.0f, 10.0f, 0.0f);
	model.rotate(m_angle, {0.0f, 1.0f, 0.0f});

    QMatrix4x4 proj;
    proj.perspective(60.0f, width() / (float)height(), 0.1f, 100.0f);

    const QMatrix4x4 matrix = proj * m_camera.get_view() * model;
	m_sphere_ub.matrix = matrix;

	m_sphere_shader->bind();
	m_sphere_shader->set_uniform_block_data((void*)&m_sphere_ub, sizeof(m_sphere_ub));

	glEnableVertexAttribArray(0); // enable attribute slot for shader input variable
	glBindBuffer(GL_ARRAY_BUFFER, m_sphere.vbos.pos);
	glVertexAttribPointer(m_sphere_shader->attributeLocation("position"), 3, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_vertices
	
	// re-direct rendering to frame buffer
	m_framebuffer->clear();

	glEnable(GL_CULL_FACE);

	// actual draw call of the shape (triangle)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere.vbos.index);
	if (m_show_wire_frame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_sphere.indices.size(), GL_UNSIGNED_INT, nullptr);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_sphere_shader->release();
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDisable(GL_CULL_FACE);


/// plane rendering
	
	m_plane_shader->bind();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_plane.vbos.pos);
	glVertexAttribPointer(m_plane_shader->attributeLocation("position"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	QMatrix4x4 plane_model;
    plane_model.rotate(90.0f, {1.0f, 0.0f, 0.0f});
    m_plane_ub.matrix = proj * m_camera.get_view() * plane_model;
	m_plane_shader->set_uniform_block_data((void*)&m_plane_ub, sizeof(m_plane_ub));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_plane.vbos.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_plane.vbos.index);
	glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_plane.indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_plane_shader->release();
	glDisableVertexAttribArray(1);
	glDisable(GL_BLEND);


	// switch back to back buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST); // fb does not have depth


/// post processing

	m_post_process_shader->bind();

	m_framebuffer->bind_color_texture();

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
	glVertexAttribPointer(m_post_process_shader->attributeLocation("position"), 2, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	m_post_process_shader->release();

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
        connect(this, &OpenGLWindow::frameSwapped, this, QOverload<>::of(&OpenGLWindow::update));
        
		update();
    }
    else
    {
        disconnect(this, &OpenGLWindow::frameSwapped, this, QOverload<>::of(&OpenGLWindow::update));
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
    Q_UNUSED(pos);
    const float t = (1.0f + std::sqrt(5.0f)) / 4.0f;

	m_sphere.add_normalized_vertex({-0.5f,  t, 0.0f});
	m_sphere.add_normalized_vertex({0.5f,  t, 0.0f});
	m_sphere.add_normalized_vertex({-0.5f, -t, 0.0f});
	m_sphere.add_normalized_vertex({0.5f, -t, 0.0f});

	m_sphere.add_normalized_vertex({0.0f, -0.5f,  t});
	m_sphere.add_normalized_vertex({0.0f,  0.5f,  t});
	m_sphere.add_normalized_vertex({0.0f, -0.5f, -t});
	m_sphere.add_normalized_vertex({0.0f,  0.5f, -t});

	m_sphere.add_normalized_vertex({t, 0.0f, -0.5f});
	m_sphere.add_normalized_vertex({t, 0.0f,  0.5f});
	m_sphere.add_normalized_vertex({-t, 0.0f, -0.5f});
	m_sphere.add_normalized_vertex({-t, 0.0f,  0.5f});

	m_sphere.indices.push_back({0, 11,  5});
	m_sphere.indices.push_back({0,  5,  1});
	m_sphere.indices.push_back({0,  1,  7});
	m_sphere.indices.push_back({0,  7, 10});
	m_sphere.indices.push_back({0, 10, 11});

	m_sphere.indices.push_back({1,  5,  9});
	m_sphere.indices.push_back({5, 11,  4});
	m_sphere.indices.push_back({11, 10,  2});
	m_sphere.indices.push_back({10,  7,  6});
	m_sphere.indices.push_back({7,  1,  8});

	m_sphere.indices.push_back({3,  9,  4});
	m_sphere.indices.push_back({3,  4,  2});
	m_sphere.indices.push_back({3,  2,  6});
	m_sphere.indices.push_back({3,  6,  8});
	m_sphere.indices.push_back({3,  8,  9});

	m_sphere.indices.push_back({4,  9,  5});
	m_sphere.indices.push_back({2,  4, 11});
	m_sphere.indices.push_back({6,  2, 10});
	m_sphere.indices.push_back({8,  6,  7});
	m_sphere.indices.push_back({9,  8,  1});

	m_sphere.sub_divide(4);
	m_sphere.scale(size);
}

void OpenGLWindow::resizeGL(int width, int height)
{
	m_framebuffer->resize(width, height);
}
