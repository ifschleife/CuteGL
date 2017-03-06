#include "mesh.h"

#include "util.h"

#include <cassert>
#include <math.h>


Mesh::Mesh()
    : m_index_buffer{0}
    , m_normal_buffer{0}
    , m_position_buffer{0}
    , m_texcoord_buffer{0}
{
    initializeOpenGLFunctions();
}

void Mesh::initVBOs()
{
    glGenBuffers(1, &m_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, m_positions.size() * sizeof(m_positions[0]), m_positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(m_indices[0]), m_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (0 < m_normals.size())
    {
        assert(m_normals.size() == m_positions.size());
        glGenBuffers(1, &m_normal_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer);
        glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(m_normals[0]), m_normals.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (0 < m_texcoords.size())
    {
        assert(m_texcoords.size() == m_positions.size());
        glGenBuffers(1, &m_texcoord_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_texcoord_buffer);
        glBufferData(GL_ARRAY_BUFFER, m_texcoords.size() * sizeof(m_texcoords[0]), m_texcoords.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Mesh::bindBuffers(const int position_location, const int normal_location, const int texcoord_location)
{
    assert(-1 != position_location);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_position_buffer);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (0 < m_normal_buffer && -1 != normal_location)
    {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, m_normal_buffer);
        glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    if (0 < m_texcoord_buffer && -1 !=texcoord_location)
    {
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, m_texcoord_buffer);
        glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
}

void Mesh::unbindBuffers()
{
    glEnableVertexAttribArray(0);
    if (0 < m_normal_buffer)
        glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::addFace(const std::array<uint32_t, 3>&& indices)
{
    m_indices.emplace_back(indices);
}

void Mesh::addVertexPosition(float x, float y, float z)
{
    m_positions.emplace_back(x, y, z);
}

void Mesh::addVertexPositions(const std::vector<Vec3D>& positions)
{
    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
}

void Mesh::addVertexNormal(float x, float y, float z)
{
    m_normals.emplace_back(x, y, z);
}

void Mesh::addVertexTexCoord(float x, float y)
{
    m_texcoords.emplace_back(x, y);
}

void Mesh::addVertexTexCoords(const std::vector<std::pair<float, float>>& coords)
{
    m_texcoords.insert(m_texcoords.end(), coords.begin(), coords.end());
}

uint32_t Mesh::addVertex(const Vec3D& vertex, const Vec3D& normal)
{
    const auto idx = static_cast<uint32_t>(m_positions.size());
    m_positions.emplace_back(vertex);
    m_normals.emplace_back(normal);
    return idx;
}

uint32_t Mesh::addNormalizedVertex(const Vec3D&& vertex)
{
    const auto idx = static_cast<uint32_t>(m_positions.size());
    m_positions.emplace_back(vertex);
    m_positions.back().normalize();
    return idx;
}

void Mesh::draw()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
    glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::scale(float factor)
{
    for (auto& vtx : m_positions)
    {
        vtx *= factor;
    }
}

void Mesh::subDivide(uint_fast8_t level)
{
    assert(level > 0);

    for (int i = 0; i < level; ++i)
    {
        std::vector<std::array<uint32_t, 3>> new_faces;

        for (const auto& face : m_indices)
        {
            const auto v1 = m_positions[face[0]];
            const auto v2 = m_positions[face[1]];
            const auto v3 = m_positions[face[2]];

            Vec3D v1_v2{(v1 + v2) / 2.0f}; v1_v2.normalize();
            const auto v1_v2_idx = addVertex(v1_v2, v1_v2);
            Vec3D v2_v3{(v2 + v3) / 2.0f}; v2_v3.normalize();
            const auto v2_v3_idx = addVertex(v2_v3, v2_v3);
            Vec3D v3_v1{(v3 + v1) / 2.0f}; v3_v1.normalize();
            const auto v3_v1_idx = addVertex(v3_v1, v3_v1);

            new_faces.push_back({face[0], v1_v2_idx, v3_v1_idx});
            new_faces.push_back({v1_v2_idx, face[1], v2_v3_idx});
            new_faces.push_back({v3_v1_idx, v2_v3_idx, face[2]});
            new_faces.push_back({v1_v2_idx, v2_v3_idx, v3_v1_idx});
        }

        m_indices = new_faces;
    }
}

std::unique_ptr<Mesh> Mesh::createSubDivSphere(float size, int level)
{
    std::unique_ptr<Mesh> sphere = std::make_unique<Mesh>();
    const float t = (1.0f + std::sqrt(5.0f)) / 4.0f;

    Vec3D vert = Vec3D{-0.5f, t, 0.0f}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{0.5f, t, 0.0f}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{-0.5f, -t, 0.0f}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{0.5f, -t, 0.0f}.normalized();
    sphere->addVertex(vert, vert);

    vert = Vec3D{0.0f, -0.5f, t}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{0.0f, 0.5f, t}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{0.0f, -0.5f, -t}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{0.0f, 0.5f, -t}.normalized();
    sphere->addVertex(vert, vert);

    vert = Vec3D{t, 0.0f, -0.5f}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{t, 0.0f, 0.5f}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{-t, 0.0f, -0.5f}.normalized();
    sphere->addVertex(vert, vert);
    vert = Vec3D{-t, 0.0f, 0.5f}.normalized();
    sphere->addVertex(vert, vert);

    sphere->addFace({0, 11, 5});
    sphere->addFace({0, 5, 1});
    sphere->addFace({0, 1, 7});
    sphere->addFace({0, 7, 10});
    sphere->addFace({0, 10, 11});

    sphere->addFace({1, 5, 9});
    sphere->addFace({5, 11, 4});
    sphere->addFace({11, 10, 2});
    sphere->addFace({10, 7, 6});
    sphere->addFace({7, 1, 8});

    sphere->addFace({3, 9, 4});
    sphere->addFace({3, 4, 2});
    sphere->addFace({3, 2, 6});
    sphere->addFace({3, 6, 8});
    sphere->addFace({3, 8, 9});

    sphere->addFace({4, 9, 5});
    sphere->addFace({2, 4, 11});
    sphere->addFace({6, 2, 10});
    sphere->addFace({8, 6, 7});
    sphere->addFace({9, 8, 1});

    sphere->subDivide(level);
    sphere->scale(size);

    return sphere;
}
