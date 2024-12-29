cmake_minimum_required(VERSION 3.5)

project(tetris3D VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        cube.cpp cube.h
        block.h block.cpp
        gamefield.h gamefield.cpp
        tetrisengine.h tetrisengine.cpp
        renderer.h renderer.cpp
	consts.h
	polygonmeshobject.h polygonmeshobject.cpp
	scene.h scene.cpp
	gamefieldside.h gamefieldside.cpp
)

set(CMAKE_C_FLAGS_RELEASE "-Os -s -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -s -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
set(CMAKE_BUILD_TYPE MinSizeRel)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(tetris3D
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tetris3D APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tetris3D SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tetris3D
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(tetris3D PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(tetris3D PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tetris3D)
endif()

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(tetris3D PRIVATE OpenMP::OpenMP_CXX)
endif()

