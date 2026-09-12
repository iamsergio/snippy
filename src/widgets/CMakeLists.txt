find_package(Qt6Widgets REQUIRED)
find_package(Qt6Core5Compat)

add_executable(snippy
    foldericondelegate.cpp
    main.cpp
    mainwindow.cpp
    textedit.cpp
    syntaxhighlighter.cpp
    mainwindow.ui
    ${CMAKE_SOURCE_DIR}/resources.qrc
    )

target_include_directories(snippy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Qt6::Qml comes in transitively via snippy_core's PUBLIC link (QJSEngine in SnippetProxyModel).
target_link_libraries(snippy PRIVATE snippy_core Qt6::Widgets Qt6::Core5Compat)
