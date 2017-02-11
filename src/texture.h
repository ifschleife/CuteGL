#pragma once

#include <memory>
#include <string>

#include <QOpenGLFunctions_4_5_Core>

class QImage;


class Texture : protected QOpenGLFunctions_4_5_Core
{
public:
    explicit Texture();

    bool loadFromFile(const std::string& filename);

    void bind();

    void setAnisotropicFilteringLevel(int level);
    void setMinMagFilters(GLint min_filter, GLint mag_filter);
    void setWrappingST(GLint s_wrapping, GLint t_wrapping);

private:
    void uploadToGPU(int nof_mipmaps);

private:
    GLuint m_id;
    std::unique_ptr<QImage> m_image;
};
