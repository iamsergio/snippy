TEMPLATE = app
TARGET = snippy
INCLUDEPATH += .

# Input
FORMS += mainwindow.ui
SOURCES += main.cpp mainwindow.cpp snippetmodel.cpp snippetproxymodel.cpp kernel.cpp snippet.cpp
HEADERS += snippetmodel.h snippetproxymodel.h mainwindow.h kernel.h snippet.h
RESOURCES += resources.qrc

QT += widgets qml
CONFIG += c++11
