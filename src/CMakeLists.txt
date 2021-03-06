file(GLOB SOURCES "*.h" "*.cpp" "*.qrc" "*.ui")

file(GLOB_RECURSE RES_FILES *.glsl) # shader code

find_package(Qt5Core REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${CMAKE_HOME_DIRECTORY}/thirdparty)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RES_FILES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::OpenGL)


# copy dlls next to compiled binary
if (WIN32)
    file(TO_CMAKE_PATH "${CMAKE_PREFIX_PATH}/bin" QtBinDirectory)

    set(QtDebugLibs Qt5Cored.dll Qt5Guid.dll Qt5OpenGLd.dll Qt5Widgetsd.dll)
    set(QtReleaseLibs Qt5Core.dll Qt5Gui.dll Qt5OpenGL.dll Qt5Widgets.dll)

    foreach(file ${QtDebugLibs})
        file(TO_CMAKE_PATH "${QtBinDirectory}/${file}" CuteLib)
        ADD_CUSTOM_COMMAND (TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CuteLib} ${ProjectDebugOut})
    endforeach()

    foreach(file ${QtReleaseLibs})
        file(TO_CMAKE_PATH "${QtBinDirectory}/${file}" CuteLib)
        ADD_CUSTOM_COMMAND (TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CuteLib} ${ProjectReleaseOut})
    endforeach()
endif()
