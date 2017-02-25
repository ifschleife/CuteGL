#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

class Mesh;
struct Vec3D;


class ObjParser
{
public:
    std::unique_ptr<Mesh> parse(const std::string& filename);

private:
    std::vector<std::array<int, 3>> m_faces;
    std::vector<Vec3D> m_vertices;
};
