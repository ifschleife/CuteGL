#pragma once

#include <array>
#include <cinttypes>
#include <vector>

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLWindow> // for GLuint

#include "shader.h"
#include "util.h"


struct GLShape
{
    GLuint index;
    GLuint pos;
    GLuint texture;
};

struct Vec3D
{
    float x;
    float y;
    float z;

    float length() const;
    void normalize();

    friend Vec3D operator+(const Vec3D& lhs, const Vec3D& rhs);
    friend Vec3D operator/(const Vec3D& lhs, const Vec3D& rhs);
    Vec3D& operator/(float d);
    Vec3D& operator*=(float m);
    Vec3D& operator*(float m);
};


struct Shape
{
    std::vector<Vec3D> positions;                 ///< vbo vertex positions
    std::vector<std::array<float, 3>> normals;    ///< vertex normals
    std::vector<std::array<uint32_t, 3>> indices; ///< vbo indices

    GLShape vbos;

    uint32_t add_normalized_vertex(const Vec3D&& vtx);
    void scale(float factor);
    void sub_divide(uint_fast8_t level);
};

using Mesh = Shape;

class RenderObject : public QOpenGLFunctions_4_3_Core
{
public:
    RenderObject()
    {
        initializeOpenGLFunctions();
    }

    void addFace(const std::array<uint32_t, 3>&& indices)
    {
        m_mesh.indices.emplace_back(indices);
    }

    void addVertex(const Vec3D&& vertex)
    {
        m_mesh.positions.emplace_back(vertex);
    }

    void initGL()
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

        m_material.create_uniform_block((void*)&m_model_matrix, sizeof(m_model_matrix));

        if (!m_material.link())
        {
            qDebug() << m_material.log();
        }
    }

    void rotate(float angle)
    {
        m_model_matrix.rotate(angle, {1.0f, 0.0f, 0.0f});
    }

    void setFragmentShader(const QString&& filename)
    {
        m_material.addShaderFromSourceFile(QOpenGLShader::Fragment, filename);
    }

    void setVertexShader(const QString&& filename)
    {
        m_material.addShaderFromSourceFile(QOpenGLShader::Vertex, filename);
    }

    void setTexture(const std::unique_ptr<QImage>& texture)
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

    void render(const QMatrix4x4& pv)
    {
        const QMatrix4x4 mvp = pv * m_model_matrix;

        m_material.bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, m_mesh.vbos.pos);
        glVertexAttribPointer(m_material.attributeLocation("position"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        m_material.set_uniform_block_data((void*)&mvp, sizeof(mvp));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_mesh.vbos.texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.vbos.index);
        glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_material.release();
        glDisableVertexAttribArray(1);
        glDisable(GL_BLEND);
    }

private:
    Mesh m_mesh;
    Shader m_material;
    QMatrix4x4 m_model_matrix;
};
