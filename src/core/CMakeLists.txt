find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml)

add_library(snippy_core STATIC
    kernel.cpp
    removeemptyfoldersproxymodel.cpp
    snippet.cpp
    snippetmodel.cpp
    snippetproxymodel.cpp
    )

set_target_properties(snippy_core PROPERTIES AUTOMOC ON)

target_include_directories(snippy_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# QStandardItemModel (Gui) and QJSEngine (Qml) appear in public headers (snippetmodel.h,
# snippetproxymodel.h), so both need to be PUBLIC for downstream targets to compile against them.
target_link_libraries(snippy_core PUBLIC Qt6::Core Qt6::Gui Qt6::Qml)
