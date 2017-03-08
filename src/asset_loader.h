#pragma once

#include <memory>
#include <vector>

class AssetLoader
{
public:
    static std::vector<std::unique_ptr<class Mesh>> loadObj(const class QString& filename);
};
