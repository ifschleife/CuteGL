#include "shape.h"

#include <cassert>
#include <math.h>


float Vec3D::length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

void Vec3D::normalize()
{
    *this = *this / length();
}

Vec3D operator+(const Vec3D& lhs, const Vec3D& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Vec3D operator/(const Vec3D& lhs, const Vec3D& rhs)
{
    return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
}

Vec3D& Vec3D::operator/(float d)
{
    assert(d != 0.0f);

    this->x /= d;
    this->y /= d;
    this->z /= d;

    return *this;
}

Vec3D& Vec3D::operator*=(float m)
{
    this->x *= m;
    this->y *= m;
    this->z *= m;

    return *this;
}

Vec3D& Vec3D::operator*(float m)
{
    return *this *= m;
}


uint32_t Shape::add_normalized_vertex(const Vec3D&& vtx)
{
    const auto idx = static_cast<uint32_t>(positions.size());
    positions.emplace_back(vtx);
    positions.back().normalize();
    return idx;
}

void Shape::scale(float factor)
{
    for (auto& vtx : positions)
    {
        vtx *= factor;
    }
}

void Shape::sub_divide(uint_fast8_t level)
{
    assert(level > 0);

    for (int i = 0; i < level; ++i)
    {
        std::vector<std::array<uint32_t, 3>> new_faces;

        for (const auto& face : indices)
        {
            const auto v1 = positions[face[0]];
            const auto v2 = positions[face[1]];
            const auto v3 = positions[face[2]];

            const auto v1_v2_idx = add_normalized_vertex(std::move((v1 + v2) / 2.0f));
            const auto v2_v3_idx = add_normalized_vertex(std::move((v2 + v3) / 2.0f));
            const auto v3_v1_idx = add_normalized_vertex(std::move((v3 + v1) / 2.0f));

            new_faces.push_back({face[0], v1_v2_idx, v3_v1_idx});
            new_faces.push_back({v1_v2_idx, face[1], v2_v3_idx});
            new_faces.push_back({v3_v1_idx, v2_v3_idx, face[2]});
            new_faces.push_back({v1_v2_idx, v2_v3_idx, v3_v1_idx});
        }

        indices = new_faces;
    }
}


RenderObject::RenderObject()
{
    initializeOpenGLFunctions();
}

void RenderObject::addFace(const std::array<uint32_t, 3>&& indices)
{
    m_mesh.indices.emplace_back(indices);
}

void RenderObject::addVertex(const Vec3D&& vertex)
{
    m_mesh.positions.emplace_back(vertex);
}

void RenderObject::setMesh(const Mesh& mesh)
{
    m_mesh = mesh;
}

void RenderObject::initGL()
{
    // vbo for vertex positions

    glGenBuffers(1, &m_mesh.vbos.pos);
    glBindBuffer(GL_ARRAY_BUFFER, m_mesh.vbos.pos);
    glBufferData(GL_ARRAY_BUFFER, m_mesh.positions.size() * sizeof(m_mesh.positions[0]), m_mesh.positions.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // vbo for vertex indices

    glGenBuffers(1, &m_mesh.vbos.index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.vbos.index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_mesh.indices.size() * sizeof(m_mesh.indices[0]), m_mesh.indices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

void RenderObject::setTexture(const std::unique_ptr<QImage>&& texture)
{
    glPixelStorei(GL_PACK_ALIGNMENT, 1); // for byte textures
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &m_mesh.vbos.texture);
    glBindTexture(GL_TEXTURE_2D, m_mesh.vbos.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texture->width(), texture->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->rgbSwapped().bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
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
    const QMatrix4x4 mvp = pv * m_model_matrix;

    m_shader.bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_mesh.vbos.pos);
    glVertexAttribPointer(m_shader.attributeLocation("position"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    m_shader.set_uniform_block_data((void*)&mvp, sizeof(mvp));

    if (m_mesh.vbos.texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_mesh.vbos.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    if (m_cull_faces)
        glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, m_show_wireframe ? GL_LINE : GL_FILL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.vbos.index);
    glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_shader.release();
    glDisableVertexAttribArray(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderObject::setAnimRotation(float angle)
{
    m_anim_rotation = angle;
}

void RenderObject::animate()
{
    rotate(m_anim_rotation);
}
