TEMPLATE = app
TARGET = snippy
INCLUDEPATH += .

# Input
FORMS += mainwindow.ui
SOURCES += main.cpp mainwindow.cpp snippetmodel.cpp snippetproxymodel.cpp kernel.cpp snippet.cpp textedit.cpp removeemptyfoldersproxymodel.cpp \
    syntaxhighlighter.cpp
HEADERS += snippetmodel.h snippetproxymodel.h mainwindow.h kernel.h snippet.h textedit.h removeemptyfoldersproxymodel.h \
    syntaxhighlighter.h
RESOURCES += resources.qrc

QT += widgets qml
CONFIG += c++11
