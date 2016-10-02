#pragma once

#if defined(NDEBUG)
	#define DEBUG_CALL(x) do { } while(0)
#else
	#define DEBUG_CALL(x) do { x; } while(0)
#endif
