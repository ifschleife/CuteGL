#pragma once

#include <memory>
#include <string>


class ObjParser
{
public:
    std::unique_ptr<class Mesh> parse(const std::string& filename);
};
