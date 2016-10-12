#include "shader.h"


Shader::Shader()
{
	initializeOpenGLFunctions();
}

bool Shader::addShaderFromSourceFile(QOpenGLShader::ShaderType type, const QString& fileName)
{
	if (!QOpenGLShaderProgram::addShaderFromSourceFile(type, fileName))
	{
		qDebug() << this->log();
		return false;
	}

	return true;
}

void Shader::create_uniform_block(void* data, size_t size)
{
	glGenBuffers(1, &m_uniform_vbo);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_vbo);
	glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Shader::set_uniform_block_data(void* data, size_t size)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniform_vbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
