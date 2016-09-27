#pragma once

#include <cinttypes>

#include <QOpenGLFunctions_4_5_Core>


class Framebuffer : protected QOpenGLFunctions_4_5_Core
{
public:
	void initialize(uint_fast16_t width, uint_fast16_t height);
	
	void bind_color_texture();
	void clear();
	void resize(uint_fast16_t width, uint_fast16_t height);

private:
	void initialize_internal(uint_fast16_t width, uint_fast16_t height);

private:
	GLuint m_fb_id;
	GLuint m_fb_col_id;
	GLuint m_fb_depth_id;
};
