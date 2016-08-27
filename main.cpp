#include "mainwindow.h"

#include <QApplication>

int main(int argv, char **argc)
{
    QApplication app(argv, argc);
    app.setWindowIcon(QIcon(":/img/snippy.png"));
    app.setStyleSheet("file:///:/style.qss");
    MainWindow window;
    window.show();
    return app.exec();
}
