#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>

#include <map>
#include <string>


class Shader : public QOpenGLShaderProgram, protected QOpenGLFunctions_4_5_Core
{
public:
    Shader();

    bool addShaderFromSourceFile(QOpenGLShader::ShaderType type, const QString& fileName);
    void create_uniform_block(void* data, size_t size);
    void set_uniform_block_data(void* data, size_t size);
    void unbind() { release(); }

private:
    GLuint m_uniform_vbo;
};
