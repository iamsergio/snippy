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

#ifndef SNIPPY_QML_BACKEND_H
#define SNIPPY_QML_BACKEND_H

#include "kernel.h"

#include <QObject>
#include <QPersistentModelIndex>
#include <QQmlEngine>

class Snippet;

// QML-facing wrapper around Kernel: exposes the same model/filter/CRUD surface that
// MainWindow drives directly, plus a "current selection" concept (MainWindow gets that for
// free from QTreeView's own selection model).
class QmlBackend : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Backend)
    QML_SINGLETON

    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(bool deepSearch READ isDeepSearch WRITE setDeepSearch NOTIFY deepSearchChanged)
    Q_PROPERTY(bool filterHasError READ filterHasError NOTIFY filterHasErrorChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY currentChanged)
    Q_PROPERTY(bool currentIsFolder READ currentIsFolder NOTIFY currentChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentChanged)
    Q_PROPERTY(QString currentTags READ currentTags NOTIFY currentChanged)
    Q_PROPERTY(QString currentContents READ currentContents NOTIFY currentChanged)
public:
    explicit QmlBackend(QObject *parent = nullptr);

    QAbstractItemModel *model() const;

    QString filterText() const;
    void setFilterText(const QString &);
    bool isDeepSearch() const;
    void setDeepSearch(bool);
    bool filterHasError() const;

    bool hasSelection() const;
    bool currentIsFolder() const;
    QString currentTitle() const;
    QString currentTags() const;
    QString currentContents() const;

    Q_INVOKABLE void setCurrentIndex(const QModelIndex &proxyIndex);
    Q_INVOKABLE void setCurrentTitle(const QString &);
    Q_INVOKABLE void setCurrentTags(const QString &);
    Q_INVOKABLE void setCurrentContents(const QString &);
    Q_INVOKABLE QModelIndex createSnippet();
    Q_INVOKABLE QModelIndex createFolder(const QString &name);
    Q_INVOKABLE void deleteCurrent();
    Q_INVOKABLE void reload();

Q_SIGNALS:
    void filterTextChanged(const QString &);
    void deepSearchChanged(bool);
    void filterHasErrorChanged(bool);
    void currentChanged();
    void loaded(int numSnippets, const QString &path);

private:
    Snippet *currentSnippet() const;
    QModelIndex parentForNewItem() const;

    Kernel m_kernel;
    QString m_filterText;
    QPersistentModelIndex m_currentIndex;
};

#endif
