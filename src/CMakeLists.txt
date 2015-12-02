set(SOURCES
	main.cpp
    openglwindow.cpp
    )

set(HEADERS
    openglwindow.h
    )

set(UI_FILES
    )

set(QRC_FILES
    # ../res/res.qrc
    )

#find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

include_directories(${QT_INCLUDES})

link_directories(${QT_LIBRARIES})

add_executable(QtOpenGL WIN32
    ${SOURCES} ${HEADERS} ${UI_FILES} ${QRC_FILES})

qt5_use_modules(QtOpenGL Widgets)
