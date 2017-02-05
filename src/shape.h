#pragma once

#include <array>
#include <cinttypes>
#include <vector>

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWindow> // for GLuint

#include "shader.h"
#include "util.h"


struct GLShape
{
    GLuint index{0};
    GLuint pos{0};
    GLuint texture{0};
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

class RenderObject : public QOpenGLFunctions_4_5_Core
{
public:
    RenderObject();

    void addFace(const std::array<uint32_t, 3>&& indices);
    void addVertex(const Vec3D&& vertex);

    void initGL();
    void animate();
    void render(const QMatrix4x4& pv);

    void rotate(float angle);
    void translate(float x, float y, float z);

    void setAnimRotation(float angle);

    void setFragmentShader(const QString&& filename);
    void setVertexShader(const QString&& filename);

    void setCullFaceMode(bool mode);
    void setMesh(const Mesh& mesh);
    void setTexture(const std::unique_ptr<QImage>&& texture);
    void setWireframeMode(bool mode);

private:
    Mesh m_mesh;
    Shader m_shader;
    QMatrix4x4 m_model_matrix;

    bool m_show_wireframe{false};
    bool m_cull_faces{false};
    float m_anim_rotation{0.0f};
};
