/*
  Copyright (c) 2015 Sergio Martins <iamsergio@gmail.com>

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

#include "snippet.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

Snippet::Snippet(QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true);
    QObject::connect(&m_timer, &QTimer::timeout, this, &Snippet::saveToFile);
}

void Snippet::setTitle(const QString &title)
{
    if (title != m_title) {
        m_title = title;
        scheduleSave();
    }
}

QString Snippet::title() const
{
    return m_title;
}

QString Snippet::absolutePath() const
{
    return m_absolutePath;
}

void Snippet::setAbsolutePath(const QString &path)
{
    if (path != m_absolutePath) {
        m_absolutePath = path;
        scheduleSave();
    }
}

QString Snippet::contents() const
{
    return m_contents;
}

void Snippet::setContents(const QString &contents)
{
    if (contents != m_contents) {
        m_contents = contents;
        scheduleSave();
    }
}

QStringList Snippet::tags() const
{
    return m_tags;
}

QString Snippet::tagsString() const
{
    return m_tags.join(QChar(';'));
}

void Snippet::setTags(const QString &tagsStr)
{
    setTags(tagsStr.split(QChar(';')));
}

void Snippet::setTags(const QStringList &tags)
{
    if (tags != m_tags) {
        m_tags = tags;
        scheduleSave();
    }
}

bool Snippet::isValid() const
{
    return !m_title.isEmpty();
}

enum {
    TitleLine = 0,
    TagsLine = 1
};

void Snippet::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << Q_FUNC_INFO << "Failed to open " << filePath << " due to " << file.errorString();
        return;
    }

    m_absolutePath = filePath;

    int i = 0;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine());

        if (i == TitleLine) {
            m_title = line.trimmed();
        } else if (i == TagsLine) {
            m_tags = line.trimmed().split(";");
        } else {
            m_contents += line;
        }

        ++i;
    }
}

bool Snippet::saveToFile() const
{
    QFile file(m_absolutePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << Q_FUNC_INFO << "Failed to save file" << m_absolutePath << "because" << file.errorString();
        return false;
    }

    QTextStream out(&file);
#ifndef OPTION_QT6
    out.setCodec("UTF-8");
#endif
    out << m_title << "\n" << tagsString() << "\n" << m_contents;

    qDebug() << Q_FUNC_INFO << "Saved" << m_absolutePath;
    return true;
}

void Snippet::scheduleSave()
{
    m_timer.start(2000);
}
