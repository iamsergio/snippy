#include "mainwindow.h"

#include <QApplication>

int main(int argv, char **argc)
{
    QApplication app(argv, argc);
    MainWindow window;
    window.show();
    return app.exec();
}