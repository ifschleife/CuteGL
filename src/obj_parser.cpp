#include "obj_parser.h"

#include "mesh.h"
#include "util.h"

#include <QDebug>
#include <QFileInfo>
#include <QString>

#include <functional>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>


namespace
{
    struct VertexHasher
    {
         std::size_t operator()(const std::array<int, 3>& vtx) const
         {
             return std::hash<int>()(vtx[0]) + std::hash<int>()(vtx[1]) + std::hash<int>()(vtx[2]);
         }
    };
}


std::vector<std::unique_ptr<Mesh>> ObjParser::parse(const QString& filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    const std::string obj_path = QFileInfo(filename).absolutePath().toUtf8().constData();

    std::string err;
    const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.toUtf8().constData(), obj_path.c_str());
    qDebug() << err.c_str();
    if (!ret)
        return {};

    std::vector<std::unique_ptr<Mesh>> meshes;
    meshes.reserve(shapes.size());

    for (size_t s = 0; s < shapes.size(); s++)
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

        const size_t face_count = shapes[s].mesh.num_face_vertices.size();
        if (face_count >= 1)
        {
            const int mat_id = shapes[s].mesh.material_ids[0]; // assuming all faces in mesh have the same material
            if (-1 != mat_id && !materials[mat_id].diffuse_texname.empty())
                mesh->setMaterial(obj_path + '/' + materials[mat_id].diffuse_texname);
        }
        std::unordered_map<std::array<int, 3>, int, VertexHasher> unique_indices;
        unique_indices.reserve(shapes[s].mesh.indices.size());

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < face_count; f++)
        {
            const int fv = shapes[s].mesh.num_face_vertices[f];
            std::array<uint32_t, 3> face;

            // Loop over vertices in the face.
            for (int v = 0; v < fv; v++)
            {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                const int gl_index = unique_indices.size();
                const std::array<int, 3> vtx_key = {idx.vertex_index, idx.texcoord_index, idx.normal_index};

                const auto result = unique_indices.emplace(vtx_key, gl_index);
                if (result.second) // vertex index was inserted = first occurence
                {
                    face[v] = gl_index;

                    const float vx = attrib.vertices[3 * idx.vertex_index + 0];
                    const float vy = attrib.vertices[3 * idx.vertex_index + 1];
                    const float vz = attrib.vertices[3 * idx.vertex_index + 2];
                    mesh->addVertexPosition(vx, vy, vz);

                    if (idx.normal_index != -1)
                    {
                        const float nx = attrib.normals[3 * idx.normal_index + 0];
                        const float ny = attrib.normals[3 * idx.normal_index + 1];
                        const float nz = attrib.normals[3 * idx.normal_index + 2];
                        mesh->addVertexNormal(nx, ny, nz);
                    }
                    if (idx.texcoord_index != -1)
                    {
                        const float tx = attrib.texcoords[2*idx.texcoord_index+0];
                        const float ty = attrib.texcoords[2*idx.texcoord_index+1];
                        mesh->addVertexTexCoord(tx, ty);
                    }
                }
                else
                {
                    // use cached index
                    face[v] = result.first->second;
                }
            }
            index_offset += fv;

            mesh->addFace(std::move(face));

        }

        meshes.push_back(std::move(mesh));
    }

    return meshes;
}
