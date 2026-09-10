cmake_minimum_required(VERSION 3.21)
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

include_directories(${CMAKE_SOURCE_DIR})
add_executable(snippy ${SNIPPY_SRCS})

find_package(Qt6Widgets REQUIRED)
find_package(Qt6Qml REQUIRED)
find_package(Qt6Core5Compat)
target_link_libraries(snippy Qt6::Widgets Qt6::Qml Qt6::Core5Compat)

option(SNIPPY_BUILD_QT_COMMANDER "Build the qt-commander injector used for UI automation" OFF)

set(QT_NO_PRIVATE_MODULE_WARNING ON)

if (SNIPPY_BUILD_QT_COMMANDER)
    set(qtcommander_dir ${CMAKE_SOURCE_DIR}/3rdparty/QtCommander)
    if (NOT EXISTS ${qtcommander_dir}/src/injector/CMakeLists.txt)
        message(FATAL_ERROR "3rdparty/QtCommander is empty — run: git submodule update --init")
    endif()

    # Read by the two subdirectories below; plain variables, not cache entries.
    set(QT_MAJOR_VERSION 6)
    set(WITH_QML OFF) # snippy is pure QWidgets

    add_subdirectory(${qtcommander_dir}/src/injector ${CMAKE_BINARY_DIR}/qt-commander/injector)
    add_subdirectory(${qtcommander_dir}/src/library ${CMAKE_BINARY_DIR}/qt-commander/library)

    # The MCP server only ever looks for the injector and the library in
    # <project root>/.qt-commander/bin (builder.py resolves BUILD_DIR relative to
    # the server's working directory), so write them straight there rather than
    # relying on a `cmake --install` step that a plain build never runs.
    set(qtcommander_bindir ${CMAKE_SOURCE_DIR}/.qt-commander/bin)
    set_target_properties(qt-injector PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${qtcommander_bindir})
    set_target_properties(qt-commander-lib PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${qtcommander_bindir})

    # snippy's directory-wide AUTOUIC/AUTORCC are inherited by the subdirectories,
    # where they are both unwanted and harmful: AUTOUIC reads the #include of
    # "ui_scanner.h" in the library's UI *scanner* as a uic form and fails looking
    # for scanner.ui. Only AUTOMOC (set by the library itself) is needed.
    set_target_properties(qt-injector qt-commander-lib PROPERTIES AUTOUIC OFF AUTORCC OFF)
endif()
