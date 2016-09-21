#include "opengl_window.h"

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QScreen>

#include <chrono>
#include <qmath.h>


namespace
{
	const float ANIMATION_SPEED = 100.0f;

    // template for overloaded function selection, courtesy of
    // http://stackoverflow.com/a/16795664/578536
    template<typename... Args> struct SELECT
    {
        template<typename C, typename R>
        static constexpr auto OVERLOAD_OF(R(C::*pmf)(Args...)) -> decltype(pmf)
        {
            return pmf;
        }
    };

	static const char* vertexShaderSource =
		"#version 450 compatibility\n"
		"in vec2 posAttr;\n"
		"in vec2 textureCoordAttr;\n"
		"out vec2 textureCoord;\n"
		"uniform mat4 matrix;\n"
		"void main() {\n"
        "   textureCoord = textureCoordAttr;\n"
        "   gl_Position = matrix * vec4(posAttr, 0.0, 1.0);\n"
        "}\n";

	static const char* fragmentShaderSource =
		"#version 450 compatibility\n"
		"in vec2 textureCoord;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"   vec4 col = texture(tex, textureCoord);\n"
        "   gl_FragColor = vec4(col.rgb, 1.0);\n"
        "}\n";

	static const char* postProcessVS =
		"#version 450 compatibility\n"
		"in vec2 posAttr;\n"
		"uniform vec2 res;\n"
		"out vec2 textureCoord;\n"
		"void main() {\n"
		"   textureCoord = (posAttr * 0.5 + vec2(0.5)) * res / vec2(1920.0, 1080.0);\n"
		"   gl_Position = vec4(posAttr, 0.0, 1.0);\n"
		"}\n";

	static const char* postProcessFS =
		"#version 450 compatibility\n"
		"in vec2 textureCoord;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"   vec4 col = texture(tex, textureCoord);\n"
		"   gl_FragColor = vec4(vec3(1.0, 1.0, 1.0) - col.rgb, 1.0); \n"
		"}\n";
}


OpenGLWindow::OpenGLWindow()
    : m_program(nullptr)
{
	m_frame_timer.setInterval(1000);

	connect(&m_frame_timer, &QTimer::timeout, this, &OpenGLWindow::updateFrameTime);
}

GLuint OpenGLWindow::loadShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();

	// shaders for shape
    m_program = new QOpenGLShaderProgram(this);
	bool shaders_ok = true;
	shaders_ok |= m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaders_ok |= m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaders_ok |= m_program->link();
	if (!shaders_ok)
	{
		qDebug() << m_program->log();
	}

    m_posAttr = m_program->attributeLocation("posAttr");
	m_textureCoordAttr = m_program->attributeLocation("textureCoordAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
	m_texture_uniform = m_program->uniformLocation("tex");

	// shaders for framebuffer quad
	m_post_process_program = new QOpenGLShaderProgram(this);
	shaders_ok = true;
	shaders_ok |= m_post_process_program->addShaderFromSourceCode(QOpenGLShader::Vertex, postProcessVS);
	shaders_ok |= m_post_process_program->addShaderFromSourceCode(QOpenGLShader::Fragment, postProcessFS);
	shaders_ok |= m_post_process_program->link();
	if (!shaders_ok)
	{
		qDebug() << m_program->log();
	}
	//char buffer[512];
	//glGetShaderInfoLog(m_program->shaders()[0]->shaderId(), 512, nullptr, buffer);
	//qDebug() << buffer;

	m_post_process_pos_attr = m_post_process_program->attributeLocation("posAttr");
	m_post_process_res_uniform = m_post_process_program->uniformLocation("res");
	m_post_process_tex_uniform = m_post_process_program->uniformLocation("tex");

	// create framebuffer color
	glGenTextures(1, &m_fb_col_id);
	glBindTexture(GL_TEXTURE_2D, m_fb_col_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// create framebuffer depth
	glGenTextures(1, &m_fb_depth_id);
	glBindTexture(GL_TEXTURE_2D, m_fb_depth_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1920, 1080, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// upload framebuffer
	glGenFramebuffers(1, &m_fb_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fb_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fb_col_id, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fb_depth_id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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


	m_shape.positions.push_back({ 0.0f,  0.7f, 1.0f});
	m_shape.positions.push_back({-0.5f, -0.5f, 1.0f});
	m_shape.positions.push_back({ 0.5f, -0.5f, 1.0f});
	m_shape.positions.push_back({ 0.5f,  0.7f, 1.0f});

	m_shape.indices.push_back({0, 1, 2});
	m_shape.indices.push_back({0, 2, 3});

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

	m_frame_timer.start();
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
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f}
	);

	QMatrix4x4 proj;
	proj.perspective(60.0f, 16.0f / 9.0f, 0.1f, 100.0f);
	proj.translate(0.0f, 0.0f, -2.0f);

	const QMatrix4x4 matrix = proj * view * model;

	m_program->bind();

	m_program->setUniformValue(m_matrixUniform, matrix);
	m_program->setUniformValue(m_texture_uniform, 0); // 0 == texture slot number from glActiveTexture

	glEnableVertexAttribArray(0); // enable attribute slot for shader input variable
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbos.pos);
	glVertexAttribPointer(m_posAttr, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_vertices
	
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_texture_coords);
	glVertexAttribPointer(m_textureCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// re-direct rendering to frame buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fb_id);
	glClear(GL_COLOR_BUFFER_BIT); // so we can work on a clean empty framebuffer

	GLfloat texture_data[] =
	{
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	texture_data[0] = (sin(time*4.0f) + 1.0f) / 2.0f;
	texture_data[5] = (sin(time*4.0f) + 1.0f) / 2.0f;
	texture_data[10] = (sin(time*4.0f) + 1.0f) / 2.0f;
	glTextureSubImage2D(m_texture_id, 0, 0, 0, 2, 2, GL_RGBA, GL_FLOAT, texture_data);

	// actual draw call of the shape (triangle)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_shape.vbos.index);
	glDrawElements(GL_TRIANGLES, 6/*3 * (GLsizei)m_shape.indices.size()*/, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	m_program->release();
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	// switch back to back buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	m_post_process_program->bind();
	m_post_process_program->setUniformValue(m_post_process_tex_uniform, 0); // 0 == texture slot number from glActiveTexture
	QRect winSize = geometry();
	m_post_process_program->setUniformValue(m_post_process_res_uniform, (float)winSize.width(), (float)winSize.height());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fb_col_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
	glVertexAttribPointer(m_post_process_pos_attr, 2, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_quad
	glDrawArrays(GL_QUADS, 0, 4);

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
        connect(this, &OpenGLWindow::frameSwapped, this, SELECT<void>::OVERLOAD_OF(&OpenGLWindow::update));
        
		update();
    }
    else
    {
        disconnect(this, &OpenGLWindow::frameSwapped, this, SELECT<void>::OVERLOAD_OF(&OpenGLWindow::update));
    }
}

void OpenGLWindow::updateFrameTime()
{
	emit frameTime(m_frame_counter / 3.60f);
	m_frame_counter = 0;
}
