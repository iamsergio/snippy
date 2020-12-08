cmake_minimum_required(VERSION 3.7)
project(snippy LANGUAGES CXX)

SET(SNIPPY_SRCS
    kernel.cpp
    main.cpp
    mainwindow.cpp
    removeemptyfoldersproxymodel.cpp
    snippet.cpp
    snippetmodel.cpp
    snippetproxymodel.cpp
    syntaxhighlighter.cpp
    textedit.cpp
    mainwindow.ui
    resources.qrc
    )

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Qml REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
add_executable(snippy ${SNIPPY_SRCS})
target_link_libraries(snippy Qt5::Widgets Qt5::Qml)
 