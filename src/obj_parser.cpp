#include "obj_parser.h"

#include "mesh.h"
#include "util.h"

#include <set>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>


struct Vertex
{
    Vertex(float p1, float p2, float p3, float n1, float n2, float n3)
      : pos{p1, p2, p3}
      , norm{n1, n2, n3}
    {
    }

    std::array<float, 3> pos;
    std::array<float, 3> norm;
    uint32_t index;

    friend bool operator<(const Vertex& lhs, const Vertex& rhs)
    {
        return std::tie(lhs.pos, lhs.norm) < std::tie(rhs.pos, rhs.norm);
    }
    bool operator==(const Vertex& other) const { return other.pos == pos && other.norm == norm; }

    bool operator!=(const Vertex& other) const { return !(*this == other); }
};


std::unique_ptr<Mesh> ObjParser::parse(const std::string& filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
    if (!ret)
        return nullptr;

    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    std::set<Vertex> unique_vertices;

    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            int fv = shapes[s].mesh.num_face_vertices[f];
            std::array<uint32_t, 3> face;

            // Loop over vertices in the face.
            for (int v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];
                float nx = attrib.normals[3 * idx.normal_index + 0];
                float ny = attrib.normals[3 * idx.normal_index + 1];
                float nz = attrib.normals[3 * idx.normal_index + 2];
                Vertex new_vert{vx, vy, vz, nx, ny, nz};
                new_vert.index = unique_vertices.size();

                const auto result = unique_vertices.insert(new_vert);
                if (result.second)
                {
                    face[v] = new_vert.index;
                    mesh->addVertex({new_vert.pos[0], new_vert.pos[1], new_vert.pos[2]},
                                    {new_vert.norm[0], new_vert.norm[1], new_vert.norm[2]});
                }
                else
                {
                    face[v] = result.first->index;
                }
                //            float tx = attrib.texcoords[2*idx.texcoord_index+0];
                //            float ty = attrib.texcoords[2*idx.texcoord_index+1];
            }
            index_offset += fv;

            mesh->addFace(std::move(face));

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    return std::move(mesh);
}
