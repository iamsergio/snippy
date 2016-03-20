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

#ifndef SNIPPET_MODEL_H
#define SNIPPET_MODEL_H

#include "snippet.h"
#include <QStandardItemModel>

class QStandardItem;
class QDir;

class SnippetModel : public QStandardItemModel
{
    Q_OBJECT
public:

    enum Role {
        SnippetRole = Qt::UserRole + 1,
        IsFolderRole,
        FolderNameRole,
        AbsolutePathRole,
        RelativePathRole
    };

    explicit SnippetModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool isFolder(const QModelIndex &index) const;
    Snippet* snippet(const QModelIndex &index) const;
    void load();
    void removeSnippet(const QModelIndex &index);
    QModelIndex addSnippet(const QModelIndex &parent);
    QStandardItem *createFolder(const QString &name, const QModelIndex &parent);

    static QString emptySnippetTitle();

Q_SIGNALS:
    void loaded(int numSnippets, const QString &path);

private:
    QStandardItem* addSnippet(Snippet *, QStandardItem *parentItem);
    QStandardItem *addFolder(const QString &name, const QString &absolutePath, QStandardItem *parentItem);
    QStandardItem * itemForName(const QString &name, const QModelIndex &parentIndex);
    void import(QDir, QStandardItem *parentItem);
    QString rootPath() const;

    int m_numSnippets;
};

#endif
