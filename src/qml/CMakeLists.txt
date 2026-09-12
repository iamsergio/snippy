find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_policy(SET QTP0001 NEW)

qt_add_library(snippy_qml_lib STATIC)
set_target_properties(snippy_qml_lib PROPERTIES AUTOMOC ON)

qt_add_qml_module(snippy_qml_lib
    URI snippy
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        qmlbackend.h
        qmlbackend.cpp
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/snippy
    )

target_link_libraries(snippy_qml_lib PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)

# PUBLIC: qmlbackend.h #includes kernel.h, and main.cpp (in the snippy-qml executable, a
# separate target) needs that transitively.
target_link_libraries(snippy_qml_lib PUBLIC snippy_core)
target_link_libraries(snippy_qml_lib PRIVATE snippy_qml_libplugin)

qt_add_executable(snippy-qml main.cpp)
set_target_properties(snippy-qml PROPERTIES AUTOMOC ON AUTORCC ON)
target_sources(snippy-qml PRIVATE ${CMAKE_SOURCE_DIR}/resources.qrc)
target_link_libraries(snippy-qml PRIVATE snippy_qml_lib Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)
