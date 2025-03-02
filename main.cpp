/*
  Copyright (c) 2015-2016 Sergio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QCommandLineParser>

static QString getArg()
{
    QStringList args = qApp->arguments();
    if (args.size() <= 1)
        return QString();

    args.removeAt(0);
    return args.join(" ");
}

int main(int argv, char **argc)
{
    QApplication app(argv, argc);
    QFont f(QStringLiteral("DejaVu Sans Mono"));
    f.setPixelSize(12);
    app.setFont(f);
    app.setWindowIcon(QIcon(":/img/snippy.png"));
    app.setStyle(QStyleFactory::create(QStringLiteral("fusion")));

    QCommandLineParser parser;
    parser.setApplicationDescription("Snippy");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption("quit-after-loading", "Quit immediately after loading (for benchmark purposes)"));
    parser.process(app);

    QString initialFilter = getArg();
    MainWindow window(initialFilter);
    window.show();

    if (parser.isSet("quit-after-loading")) {
        QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
    }

    return app.exec();
}
