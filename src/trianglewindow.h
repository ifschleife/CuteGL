#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLWindow>


class QOpenGLShaderProgram;


class TriangleWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
    TriangleWindow();

    void initializeGL() override;
    void paintGL() override;

public slots:
    void setAnimating(bool animating);

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;
};
