#pragma once

#include <QtGlobal>

#include <memory>


#if defined(NDEBUG)
	#define DEBUG_CALL(x) do { } while(0)
#else
	#define DEBUG_CALL(x) do { x; } while(0)
#endif


auto generate_checker_board_texture(int width, int height) -> std::unique_ptr<class QImage>;
