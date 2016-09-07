#include "trianglewindow.h"

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QScreen>


#include <chrono>
#include <qmath.h>


namespace
{
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

    static const char *vertexShaderSource =
        "#version 450 compatibility\n"
        "in vec2 posAttr;\n"
        "in vec2 textureCoordAttr;\n"
        "out vec2 textureCoord;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   textureCoord = textureCoordAttr;\n"
        "   gl_Position = matrix * vec4(posAttr, 0.0, 1.0);\n"
        "}\n";

	static const char *fragmentShaderSource =
		"#version 450 compatibility\n"
		"in vec2 textureCoord;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"   vec4 col = texture(tex, textureCoord);\n"
        "   gl_FragColor = vec4(col.rgb, 1.0);\n"
        "}\n";
}


TriangleWindow::TriangleWindow()
    : m_program(nullptr)
    , m_frame(0)
{
}

GLuint TriangleWindow::loadShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void TriangleWindow::initializeGL()
{
    initializeOpenGLFunctions();

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
	m_textureCoordAttr = m_program->attributeLocation("textureCoordAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
	m_texture_uniform = m_program->uniformLocation("tex");

	GLfloat vertices[] = {
		0.0f, 0.707f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};

	glGenBuffers(1, &m_vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint indices[] = { 0, 1, 2 };
	glGenBuffers(1, &m_vbo_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLfloat texture_coords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};
	glGenBuffers(1, &m_vbo_texture_coords);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_texture_coords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coords), texture_coords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLfloat texture_data[] = {
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	glGenTextures(1, &m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2, 2, 0, GL_RGBA, GL_FLOAT, texture_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TriangleWindow::paintGL()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, static_cast<GLsizei>(width() * retinaScale), static_cast<GLsizei>(height() * retinaScale));

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix; matrix.perspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f); matrix.translate(0, 0, -2);
    matrix.rotate(100.0f * m_frame / static_cast<float>(screen()->refreshRate()), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);
	m_program->setUniformValue(m_texture_uniform, 0); // 0 == texture slot number from glActiveTexture

	glEnableVertexAttribArray(0); // enable attribute slot for shader input variable
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertices);
	glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, nullptr); // nullptr == user currently bound buffer e.g. m_vbo_vertices
	
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_texture_coords);
	glVertexAttribPointer(m_textureCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	// actual draw call
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0); // make sure we unbind the correct texture slot
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

    m_program->release();

    ++m_frame;
}

void TriangleWindow::setAnimating(bool animating)
{
    if (animating)
    {
        // Animate continuously, throttled by the blocking swapBuffers() call the
        // QOpenGLWindow internally executes after each paint. Once that is done
        // (frameSwapped signal is emitted), we schedule a new update. This
        // obviously assumes that the swap interval (see
        // QSurfaceFormat::setSwapInterval()) is non-zero.
        connect(this, &TriangleWindow::frameSwapped, this, SELECT<void>::OVERLOAD_OF(&TriangleWindow::update));
        update();
    }
    else
    {
        disconnect(this, &TriangleWindow::frameSwapped, this, SELECT<void>::OVERLOAD_OF(&TriangleWindow::update));
    }
}
