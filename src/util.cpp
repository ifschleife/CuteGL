#include "util.h"

#include "shape.h"

#include <QImage>


float deg_to_rad(float degrees) { return degrees * 4.0f * atanf(1.0f) / 180.0f; }

Shape createSubDivSphere(float size, int level)
{
    Shape sphere;
    const float t = (1.0f + std::sqrt(5.0f)) / 4.0f;

    sphere.add_normalized_vertex({-0.5f, t, 0.0f});
    sphere.add_normalized_vertex({0.5f, t, 0.0f});
    sphere.add_normalized_vertex({-0.5f, -t, 0.0f});
    sphere.add_normalized_vertex({0.5f, -t, 0.0f});

    sphere.add_normalized_vertex({0.0f, -0.5f, t});
    sphere.add_normalized_vertex({0.0f, 0.5f, t});
    sphere.add_normalized_vertex({0.0f, -0.5f, -t});
    sphere.add_normalized_vertex({0.0f, 0.5f, -t});

    sphere.add_normalized_vertex({t, 0.0f, -0.5f});
    sphere.add_normalized_vertex({t, 0.0f, 0.5f});
    sphere.add_normalized_vertex({-t, 0.0f, -0.5f});
    sphere.add_normalized_vertex({-t, 0.0f, 0.5f});

    sphere.indices.push_back({0, 11, 5});
    sphere.indices.push_back({0, 5, 1});
    sphere.indices.push_back({0, 1, 7});
    sphere.indices.push_back({0, 7, 10});
    sphere.indices.push_back({0, 10, 11});

    sphere.indices.push_back({1, 5, 9});
    sphere.indices.push_back({5, 11, 4});
    sphere.indices.push_back({11, 10, 2});
    sphere.indices.push_back({10, 7, 6});
    sphere.indices.push_back({7, 1, 8});

    sphere.indices.push_back({3, 9, 4});
    sphere.indices.push_back({3, 4, 2});
    sphere.indices.push_back({3, 2, 6});
    sphere.indices.push_back({3, 6, 8});
    sphere.indices.push_back({3, 8, 9});

    sphere.indices.push_back({4, 9, 5});
    sphere.indices.push_back({2, 4, 11});
    sphere.indices.push_back({6, 2, 10});
    sphere.indices.push_back({8, 6, 7});
    sphere.indices.push_back({9, 8, 1});

    sphere.sub_divide(level);
    sphere.scale(size);

    return sphere;
}

auto generate_checker_board_texture(int width, int height) -> std::unique_ptr<QImage>
{
    auto img = std::make_unique<QImage>(width, height, QImage::Format_RGB32);

    bool white = true;
    for (int y = 0; y < height; ++y)
    {
        uchar* line = img->scanLine(y);
        for (int x = 0; x < width * 4; x += 4)
        {
            line[x] = white ? 255 : 0;
            line[x + 1] = white ? 255 : 0;
            line[x + 2] = white ? 255 : 0;
            line[x + 3] = 255; // alpha
            white = !white;
        }
        white = !white; // for next line
    }

    return img;
}
