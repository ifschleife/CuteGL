#include "framebuffer.h"


void Framebuffer::initialize(uint_fast16_t width, uint_fast16_t height)
{
	initializeOpenGLFunctions();

	initialize_internal(width, height);
}

void Framebuffer::bind_color_texture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fb_col_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Framebuffer::clear()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fb_id);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // so we can work on a clean empty framebuffer
	glEnable(GL_DEPTH_TEST);
}

void Framebuffer::resize(uint_fast16_t width, uint_fast16_t height)
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glDeleteTextures(1, &m_fb_col_id);
	glDeleteTextures(1, &m_fb_depth_id);
	glDeleteFramebuffers(1, &m_fb_id);

	initialize_internal(width, height);
}


void Framebuffer::initialize_internal(uint_fast16_t width, uint_fast16_t height)
{
	// create framebuffer color
	glGenTextures(1, &m_fb_col_id);
	glBindTexture(GL_TEXTURE_2D, m_fb_col_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// create framebuffer depth
	glGenTextures(1, &m_fb_depth_id);
	glBindTexture(GL_TEXTURE_2D, m_fb_depth_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	// upload framebuffer
	glGenFramebuffers(1, &m_fb_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fb_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fb_col_id, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fb_depth_id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
