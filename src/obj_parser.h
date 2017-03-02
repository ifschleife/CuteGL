#pragma once

#include <memory>
#include <string>
#include <vector>


class ObjParser
{
public:
    std::vector<std::unique_ptr<class Mesh>> parse(const std::string& filename);
};
