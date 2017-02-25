#pragma once

#include <QOpenGLFunctions_4_5_Core>

#include <array>
#include <cinttypes>
#include <memory>
#include <vector>

#include <math.h>

struct Vec3D;


class Mesh : protected QOpenGLFunctions_4_5_Core
{
public:
    explicit Mesh();

    void initVBOs();

    void bind();
    void bind_normals();
    void unbind();

    void addFace(const std::array<uint32_t, 3>&& indices);
    void addVertex(const Vec3D&& vertex);
    int addVertex(const Vec3D&& vertex, const Vec3D&& normal);
    int getVertexIndex(const Vec3D& vertex) const;
    uint32_t addNormalizedVertex(const Vec3D&& vertex);
    void draw();
    void scale(float factor);
    void subDivide(uint_fast8_t level);

    static std::unique_ptr<Mesh> createSubDivSphere(float size, int level);

private:
    GLuint m_index_id;                              ///< gl id for vbo indices
    std::vector<std::array<uint32_t, 3>> m_indices; ///< vbo indices

    GLuint m_normal_id;
    std::vector<Vec3D> m_normals;

    GLuint m_vertex_id;            ///< gl id for vbo vertices
    std::vector<Vec3D> m_vertices; ///< vbo vertex positions
};
