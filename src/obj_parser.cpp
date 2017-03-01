#include "obj_parser.h"

#include "mesh.h"
#include "util.h"

#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>


std::unique_ptr<Mesh> ObjParser::parse(const std::string& filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str()))
        return nullptr;

    for (size_t s = 0; s < shapes.size(); s++)
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
        std::unordered_map<int, int> unique_indices;
        unique_indices.reserve(shapes[s].mesh.indices.size());

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            const int fv = shapes[s].mesh.num_face_vertices[f];
            std::array<uint32_t, 3> face;

            // Loop over vertices in the face.
            for (int v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                const int gl_index = unique_indices.size();
                const int hash = 100*idx.vertex_index + 10* idx.texcoord_index + idx.normal_index;

                const auto result = unique_indices.emplace(gl_index, hash);
                if (result.second)
                {
                    face[v] = gl_index;

                    const float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    const float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    const float vz = attrib.vertices[3 * idx.vertex_index + 2];
                    const float nx = attrib.normals[3 * idx.normal_index + 0];
                    const float ny = attrib.normals[3 * idx.normal_index + 1];
                    const float nz = attrib.normals[3 * idx.normal_index + 2];
                    mesh->addVertex({vx, vy, vz}, {nx, ny, nz});
                }
                else
                {
                    face[v] = result.first->second;
                }
            }
            index_offset += fv;

            mesh->addFace(std::move(face));

            // per-face material
            shapes[s].mesh.material_ids[f];
        }

        return std::move(mesh);
    }

    return nullptr;
}
