#include "mainwindow.h"

#include <QApplication>

int main(int argv, char **argc)
{
    QApplication app(argv, argc);
    app.setWindowIcon(QIcon(":/img/snippy.png"));
    MainWindow window;
    window.show();
    return app.exec();
}
