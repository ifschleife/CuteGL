#pragma once

#include <array>
#include <cinttypes>
#include <memory>
#include <vector>

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWindow> // for GLuint

#include "shader.h"
#include "util.h"


struct Shape
{
    std::vector<Vec3D> positions;                 ///< vbo vertex positions
    std::vector<std::array<uint32_t, 3>> indices; ///< vbo indices

    GLuint m_vertex_id;  ///< gl id for vbo vertices
    GLuint m_index_id;  ///< gl id for vbo indices

    void addFace(const std::array<uint32_t, 3>&& indices);
    void addVertex(const Vec3D&& vertex);
    uint32_t add_normalized_vertex(const Vec3D&& vtx);
    void scale(float factor);
    void sub_divide(uint_fast8_t level);
};

using Mesh = Shape;
class Texture;

class RenderObject : public QOpenGLFunctions_4_5_Core
{
public:
    RenderObject();

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
    void setTexture(std::unique_ptr<Texture> texture);
    void setWireframeMode(bool mode);

private:
    Mesh m_mesh;
    Shader m_shader;
    QMatrix4x4 m_model_matrix;
    std::unique_ptr<Texture> m_texture;

    bool m_show_wireframe{false};
    bool m_cull_faces{false};
    float m_anim_rotation{0.0f};
};
