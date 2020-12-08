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

option(OPTION_QT6 "Build against Qt6" OFF)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_SOURCE_DIR})
add_executable(snippy ${SNIPPY_SRCS})

if (OPTION_QT6)
    find_package(Qt6Widgets REQUIRED)
    find_package(Qt6Qml REQUIRED)
    find_package(Qt6Core5Compat)
    target_link_libraries(snippy Qt6::Widgets Qt6::Qml Qt6::Core5Compat)
    add_definitions(-DOPTION_QT6)
else()
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Qml REQUIRED)
    target_link_libraries(snippy Qt5::Widgets Qt5::Qml)
endif()    
 