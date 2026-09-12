/*
  Copyright (c) 2026 Sergio Martins <iamsergio@gmail.com>

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

#include "qmlbackend.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>

#if defined(Q_OS_LINUX) && defined(SNIPPY_DEVELOPER_BUILD)
#include <sys/prctl.h>
#endif

static QString getArg()
{
    QStringList args = qApp->arguments();
    if (args.size() <= 1)
        return QString();

    args.removeAt(0);
    return args.join(" ");
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

#if defined(Q_OS_LINUX) && defined(SNIPPY_DEVELOPER_BUILD)
    // See src/widgets/main.cpp for why this is needed.
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
#endif

    app.setWindowIcon(QIcon(":/img/snippy.png"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Snippy");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption("quit-after-loading", "Quit immediately after loading (for benchmark purposes)"));
    parser.process(app);

    QQmlApplicationEngine engine;
    engine.loadFromModule("snippy", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    auto *backend = engine.singletonInstance<QmlBackend *>("snippy", "Backend");
    backend->setFilterText(getArg());

    if (parser.isSet("quit-after-loading")) {
        QObject::connect(backend, &QmlBackend::loaded, &app, &QGuiApplication::quit);
    }

    return app.exec();
}
