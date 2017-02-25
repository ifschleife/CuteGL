#include "mesh.h"

#include "util.h"

#include <cassert>


Mesh::Mesh()
    : m_index_id{0}
    , m_normal_id{0}
    , m_vertex_id{0}
{
    initializeOpenGLFunctions();
}

void Mesh::initVBOs()
{
    glGenBuffers(1, &m_vertex_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_id);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_index_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(m_indices[0]), m_indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenBuffers(1, &m_normal_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_normal_id);
    glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(m_normals[0]), m_normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_id);
}

void Mesh::bind_normals()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_normal_id);
}

void Mesh::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::addFace(const std::array<uint32_t, 3>&& indices)
{
    m_indices.emplace_back(indices);
}

void Mesh::addVertex(const Vec3D&& vertex)
{
    m_vertices.emplace_back(vertex);
}

int Mesh::addVertex(const Vec3D&& vertex, const Vec3D&& normal)
{
    m_vertices.emplace_back(vertex);
    m_normals.emplace_back(normal);
    return m_vertices.size();
}

int Mesh::getVertexIndex(const Vec3D& vertex) const
{
    for (size_t i=0; i<m_vertices.size(); ++i)
    {
        if (m_vertices[i] == vertex)
            return i;
    }
    return -1;
}

uint32_t Mesh::addNormalizedVertex(const Vec3D&& vertex)
{
    const auto idx = static_cast<uint32_t>(m_vertices.size());
    m_vertices.emplace_back(vertex);
    m_vertices.back().normalize();
    return idx;
}

void Mesh::draw()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_id);
    glDrawElements(GL_TRIANGLES, 3 * (GLsizei)m_indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::scale(float factor)
{
    for (auto& vtx : m_vertices)
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
            const auto v1 = m_vertices[face[0]];
            const auto v2 = m_vertices[face[1]];
            const auto v3 = m_vertices[face[2]];

            const auto v1_v2_idx = addNormalizedVertex(std::move((v1 + v2) / 2.0f));
            const auto v2_v3_idx = addNormalizedVertex(std::move((v2 + v3) / 2.0f));
            const auto v3_v1_idx = addNormalizedVertex(std::move((v3 + v1) / 2.0f));

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

    sphere->addNormalizedVertex({-0.5f, t, 0.0f});
    sphere->addNormalizedVertex({0.5f, t, 0.0f});
    sphere->addNormalizedVertex({-0.5f, -t, 0.0f});
    sphere->addNormalizedVertex({0.5f, -t, 0.0f});

    sphere->addNormalizedVertex({0.0f, -0.5f, t});
    sphere->addNormalizedVertex({0.0f, 0.5f, t});
    sphere->addNormalizedVertex({0.0f, -0.5f, -t});
    sphere->addNormalizedVertex({0.0f, 0.5f, -t});

    sphere->addNormalizedVertex({t, 0.0f, -0.5f});
    sphere->addNormalizedVertex({t, 0.0f, 0.5f});
    sphere->addNormalizedVertex({-t, 0.0f, -0.5f});
    sphere->addNormalizedVertex({-t, 0.0f, 0.5f});

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
