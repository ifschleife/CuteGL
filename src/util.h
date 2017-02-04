#pragma once

#include <QtGlobal>

#include <math.h>
#include <memory>


#if defined(NDEBUG)
#define DEBUG_CALL(x)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (0)
#else
#define DEBUG_CALL(x)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        x;                                                                                                             \
    } while (0)
#endif


class Shape createSubDivSphere(float size, int level);

float deg_to_rad(float degrees);

auto generate_checker_board_texture(int width, int height) -> std::unique_ptr<class QImage>;
