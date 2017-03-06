#pragma once

#include <QOpenGLFunctions_4_5_Core>

#include <array>
#include <cinttypes>
#include <memory>
#include <string>
#include <vector>

struct Vec3D;


class Mesh : protected QOpenGLFunctions_4_5_Core
{
public:
    explicit Mesh();

    void initVBOs();

    void bindBuffers(const int position_location, const int normal_location, const int texcoord_location);
    void unbindBuffers();

    void addFace(const std::array<uint32_t, 3>&& indices);
    void addVertexPosition(float x, float y, float z);
    void addVertexPositions(const std::vector<Vec3D>& positions);
    void addVertexNormal(float x, float y, float z);
    void addVertexTexCoord(float x, float y);
    void addVertexTexCoords(const std::vector<std::pair<float, float>>& coords);
    uint32_t addVertex(const Vec3D& vertex, const Vec3D& normal);
    uint32_t addNormalizedVertex(const Vec3D&& vertex);
    void draw();
    uint32_t getFaceCount() const { return m_indices.size(); }
    std::string getMaterial() const { return m_material; }
    uint32_t getVertexCount() const { return m_positions.size(); }
    void scale(float factor);
    void setMaterial(const std::string& material) { m_material = material; }
    void subDivide(uint_fast8_t level);

    static std::unique_ptr<Mesh> createSubDivSphere(float size, int level);

private:
    GLuint m_index_buffer;                          ///< gl id for vbo indices
    std::vector<std::array<uint32_t, 3>> m_indices; ///< vbo indices

    std::string m_material;

    GLuint m_normal_buffer;
    std::vector<Vec3D> m_normals;

    GLuint m_position_buffer;       ///< gl id for vbo vertices
    std::vector<Vec3D> m_positions; ///< vbo vertex positions

    GLuint m_texcoord_buffer;
    std::vector<std::pair<float, float>> m_texcoords;
};
