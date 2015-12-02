cmake_minimum_required(VERSION 3.0)
project(QtOpenGL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

if(WIN32)
	if(MSVC11)
		set(CMAKE_PREFIX_PATH "C://Qt//Qt5.5.0//5.5//msvc2012_opengl" ${CMAKE_PREFIX_PATH})
	endif()

	set(CMAKE_PREFIX_PATH "C://Program Files (x86)//Windows Kits//8.0//Lib//win8//um//x64" ${CMAKE_PREFIX_PATH})
elseif(UNIX)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Werror -pedantic -pedantic-errors -Weffc++ \
    #                                        -Wfloat-conversion")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Werror -pedantic -pedantic-errors")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src)
