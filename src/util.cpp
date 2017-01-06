#include "util.h"

#include <QImage>


float deg_to_rad(float degrees) { return degrees * 4.0f * atanf(1.0f) / 180.0f; }

auto generate_checker_board_texture(int width, int height) -> std::unique_ptr<QImage>
{
    auto img = std::make_unique<QImage>(width, height, QImage::Format_ARGB32);

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
