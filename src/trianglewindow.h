#include "openglwindow.h"

#include <memory>


class QOpenGLShaderProgram;


class TriangleWindow : public OpenGLWindow
{
public:
    TriangleWindow();

    void initialize() override;
    void render() override;

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

private:
    QOpenGLShaderProgram* m_program;
    int m_frame;
};

