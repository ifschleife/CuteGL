cmake_minimum_required(VERSION 3.0)
project(CuteGL)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

set(CMAKE_CXX_STANDARD 14)

if(WIN32)
	file(TO_CMAKE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug" ProjectDebugOut)
	file(MAKE_DIRECTORY "${ProjectDebugOut}")
	file(TO_CMAKE_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release" ProjectReleaseOut)
	file(MAKE_DIRECTORY "${ProjectReleaseOut}")

    set(CMAKE_PREFIX_PATH "C://Qt//Qt5.9.1//5.9.1//msvc2017_64" ${CMAKE_PREFIX_PATH})

elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic -pedantic-errors \
                                            -Wfloat-conversion")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(src)
