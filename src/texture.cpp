#include "texture.h"

#include <QImage>


Texture::Texture()
    : m_id{0}
    , m_image{nullptr}
{
    initializeOpenGLFunctions();
}

void Texture::bind()
{
    glBindTextureUnit(0, m_id);
}

bool Texture::loadFromFile(const std::string& filename)
{
    m_image = std::make_unique<QImage>(filename.c_str());
    if (!m_image->isNull())
    {
        uploadToGPU(4);
        m_image.release(); // we don't need it in main memory anymore
        return 0 < m_id;
    }
    return false;
}

void Texture::uploadToGPU(int nof_mipmaps)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, nof_mipmaps, GL_RGBA8, m_image->width(), m_image->height());
    glTextureSubImage2D(m_id, 0, 0, 0, m_image->width(), m_image->height(),
                        GL_RGBA, GL_UNSIGNED_BYTE, m_image->rgbSwapped().bits());

    if (0 < nof_mipmaps)
    {
        glGenerateTextureMipmap(m_id);
    }
}

void Texture::setAnisotropicFilteringLevel(int level)
{
    glTextureParameteri(m_id, GL_TEXTURE_MAX_ANISOTROPY_EXT, level);
}

void Texture::setMinMagFilters(GLint min_filter, GLint mag_filter)
{
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, min_filter);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void Texture::setWrappingST(GLint s_wrapping, GLint t_wrapping)
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, s_wrapping);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, t_wrapping);
}
