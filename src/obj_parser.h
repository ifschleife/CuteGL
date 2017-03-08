#pragma once

#include <memory>
#include <vector>

class ObjParser
{
public:
    std::vector<std::unique_ptr<class Mesh>> parse(const class QString& filename) const;
};
