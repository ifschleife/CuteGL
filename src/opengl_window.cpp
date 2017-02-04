#include "opengl_window.h"

#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QTimer>
#include <QtMath>

#include <chrono>
#include <math.h>
#include <stdexcept>

#include "framebuffer.h"
#include "shader.h"
#include "util.h"


namespace
{
    const float ANIMATION_SPEED = 5.0f;
    const uint_fast16_t RESOLUTION_WIDTH = 1920;
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

    // shaders for framebuffer quad
    m_post_process_shader = std::make_unique<Shader>();
    m_post_process_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "../src/shaders/post_process_vs.glsl");
    m_post_process_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "../src/shaders/post_process_fs.glsl");
    if (!m_post_process_shader->link())
    {
        qDebug() << m_post_process_shader->log();
    }
    // char buffer[512];
    // glGetShaderInfoLog(m_program->shaders()[0]->shaderId(), 512, nullptr, buffer);
    // qDebug() << buffer;

    m_post_process_shader->attributeLocation("position");
    m_post_process_shader->uniformLocation("tex");

    // vbo for a quad
    GLfloat quad[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    glGenBuffers(1, &m_vbo_quad);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    {
        /// create sphere
        auto sphere = std::make_unique<RenderObject>();
        sphere->translate(0.0f, 10.0f, 0.0f);
        sphere->setAnimRotation(5.0f);

        sphere->setCullFaceMode(true);

        sphere->setMesh(createSubDivSphere(0.5f, 4));

        sphere->setVertexShader("../src/shaders/normal_vs.glsl");
        sphere->setFragmentShader("../src/shaders/normal_fs.glsl");

        m_objects.push_back(std::move(sphere));
    }

    {
        /// create plane
        auto plane = std::make_unique<RenderObject>();

        plane->rotate(90.0f);

        plane->addVertex({-8.0f, 0.0f, -8.0f});
        plane->addVertex({8.0f, 0.0f, -8.0f});
        plane->addVertex({8.0f, 0.0f, 8.0f});
        plane->addVertex({-8.0f, 0.0f, 8.0f});

        plane->addFace({0, 1, 2});
        plane->addFace({2, 3, 0});

        plane->setVertexShader("../src/shaders/plane_vs.glsl");
        plane->setFragmentShader("../src/shaders/texture_fs.glsl");

        plane->setTexture(generate_checker_board_texture(16, 16));

        m_objects.push_back(std::move(plane));
    }

    for (auto& obj: m_objects)
    {
        obj->initGL();
    }

    m_frame_timer->start();
}


void OpenGLWindow::paintGL()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, static_cast<GLsizei>(width() * retinaScale), static_cast<GLsizei>(height() * retinaScale));

    QMatrix4x4 proj;
    proj.perspective(60.0f, width() / (float)height(), 0.1f, 100.0f);

    // re-direct rendering to frame buffer
    m_framebuffer->clear();

    /// object rendering
    {
        const QMatrix4x4 pv = proj * m_camera.get_view();

        for (auto& object: m_objects)
        {
            if (m_animating)
                object->animate();
            object->render(pv);
        }
    }


    // switch back to back buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // fb does not have depth


    /// post processing

    m_post_process_shader->bind();

    m_framebuffer->bind_color_texture();

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_quad);
    glVertexAttribPointer(m_post_process_shader->attributeLocation("position"), 2, GL_FLOAT, GL_FALSE, 0,
                          nullptr); // nullptr == uses currently bound buffer e.g. m_vbo_quad
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
}

void OpenGLWindow::showWireFrame(bool status)
{
    for (auto& object: m_objects)
    {
        object->setWireframeMode(status);
    }
    requestUpdate();
}

void OpenGLWindow::updateFrameTime()
{
    emit frameTime(1000.0f / m_frame_counter);
    m_frame_counter = 0;
}


void OpenGLWindow::handle_log_message(const QOpenGLDebugMessage& msg)
{
//    qDebug() << msg;
    if (msg.severity() == QOpenGLDebugMessage::HighSeverity)
    {
        throw std::runtime_error(msg.message().toStdString());
    }
}

void OpenGLWindow::resizeGL(int width, int height)
{
    m_framebuffer->resize(width, height);
}
