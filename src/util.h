#pragma once

#if defined(NDEBUG)
	#define DEBUG_CALL(x) do { } while(0)
#else
	#define DEBUG_CALL(x) do { x; } while(0)
#endif

// template for overloaded function selection, courtesy of
// http://stackoverflow.com/a/16795664/578536
template<typename... Args> struct SELECT
{
	template<typename C, typename R>
	static constexpr auto OVERLOAD_OF(R(C::*pmf)(Args...)) -> decltype(pmf)
	{
		return pmf;
	}
};
