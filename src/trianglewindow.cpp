#include "trianglewindow.h"

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QScreen>

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
        "in vec3 colAttr;\n"
        "out vec3 col;\n"
        "uniform mat4 matrix;\n"
        "void main() {\n"
        "   col = colAttr;\n"
        "   gl_Position = matrix * vec4(posAttr, 0.0, 1.0);\n"
        "}\n";

    static const char *fragmentShaderSource =
        "#version 450 compatibility\n"
        "in vec3 col;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(col, 1.0);\n"
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
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
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

    GLfloat vertices[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 3);

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
