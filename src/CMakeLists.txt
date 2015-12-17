set(SOURCES
    main.cpp
    mainwindow.cpp
    openglwindow.cpp
    trianglewindow.cpp
    )

set(HEADERS
    mainwindow.h
    openglwindow.h
    trianglewindow.h
    )

set(UI_FILES
    mainwindow.ui
    )

set(QRC_FILES
    )

find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)

include_directories(${QT_INCLUDES})

link_directories(${QT_LIBRARIES})

add_executable(CuteGL WIN32
    ${SOURCES} ${HEADERS} ${UI_FILES} ${QRC_FILES})

qt5_use_modules(CuteGL Widgets)
