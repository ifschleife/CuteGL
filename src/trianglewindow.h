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
	GLuint m_textureCoordAttr;
    GLuint m_matrixUniform;
	
	GLuint m_index_buffer;
	GLuint m_vbo_vertices;
	GLuint m_vbo_indices;
	GLuint m_vbo_texture_coords;
	GLuint m_texture_id;
	GLuint m_texture_uniform;

	GLuint m_fb_id;
	GLuint m_fb_col_id;
	GLuint m_fb_depth_id;

    QOpenGLShaderProgram* m_program;
	GLuint m_post_process_pos_attr;
	GLuint m_post_process_res_uniform;
	GLuint m_post_process_tex_uniform;

	GLuint m_vbo_quad;

	QOpenGLShaderProgram* m_post_process_program;

    int m_frame;
};
