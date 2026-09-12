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

#ifndef SNIPPY_SNIPPET_H
#define SNIPPY_SNIPPET_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTimer>

class Snippet : public QObject
{
    Q_OBJECT
public:
    explicit Snippet(const QString &absoluteFileName,
                     QObject *parent = nullptr);

    QString title() const;
    void setTitle(const QString &);

    QString absolutePath() const;

    QString contents() const;
    void setContents(const QString &);

    QStringList tags() const;
    QString tagsString() const;
    void setTags(const QString &tagsStr);
    void setTags(const QStringList &tags);
    bool isValid() const;

    void loadFromFile();
    bool saveToFile() const;

private:
    void scheduleSave();
    const QString m_absolutePath;
    QTimer m_timer;
    QString m_title;
    QString m_contents;
    QStringList m_tags;
};

#endif
