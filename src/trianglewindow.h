#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Compatibility>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWindow>


class QOpenGLShaderProgram;


class TriangleWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
public:
    TriangleWindow();

    void initializeGL() override;
    void paintGL() override;

public slots:
    void setAnimating(bool animating);

private:
    GLuint loadShader(GLenum type, const char* source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram* m_program;
    int m_frame;
};
