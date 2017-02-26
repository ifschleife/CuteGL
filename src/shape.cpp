#include "shape.h"

#include "texture.h"

#include <cassert>


RenderObject::RenderObject()
    : m_mesh{nullptr}
    , m_texture{nullptr}
{
    initializeOpenGLFunctions();
}

void RenderObject::setMesh(std::unique_ptr<Mesh> mesh)
{
    m_mesh = std::move(mesh);
}

void RenderObject::initGL()
{
    m_mesh->initVBOs();

    m_shader.create_uniform_block((void*)&m_model_matrix, sizeof(m_model_matrix));

    if (!m_shader.link())
    {
        qDebug() << m_shader.log();
    }
}

void RenderObject::rotate(float angle)
{
    m_model_matrix.rotate(angle, {1.0f, 0.0f, 0.0f});
}

void RenderObject::translate(float x, float y, float z)
{
    m_model_matrix.translate(x, y, z);
}

void RenderObject::setFragmentShader(const QString&& filename)
{
    m_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, filename);
}

void RenderObject::setVertexShader(const QString&& filename)
{
    m_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, filename);
}

void RenderObject::setTexture(std::unique_ptr<Texture> texture)
{
    m_texture = std::move(texture);
}

void RenderObject::setCullFaceMode(bool mode)
{
    m_cull_faces = mode;
}

void RenderObject::setWireframeMode(bool mode)
{
    m_show_wireframe = mode;
}

void RenderObject::render(const QMatrix4x4& pv)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_cull_faces)
        glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, m_show_wireframe ? GL_LINE : GL_FILL);

    m_shader.bind();
    const int position_location = m_shader.attributeLocation("position");
    const int normal_location = m_shader.attributeLocation("normal_in");
    m_mesh->bindBuffers(position_location, normal_location);

    const QMatrix4x4 mvp = pv * m_model_matrix;
    m_shader.set_uniform_block_data((void*)&mvp, sizeof(mvp));

    if (m_texture)
    {
        m_texture->bind();
        m_mesh->draw();
        m_texture->unbind();
    }
    else
    {
        m_mesh->draw();
    }

    m_mesh->unbindBuffers();
    m_shader.unbind();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void RenderObject::setAnimRotation(float angle)
{
    m_anim_rotation = angle;
}

void RenderObject::animate()
{
    rotate(m_anim_rotation);
}
