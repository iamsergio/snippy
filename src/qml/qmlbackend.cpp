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
#include "snippet.h"
#include "snippetmodel.h"
#include "snippetproxymodel.h"

#include <QStandardItem>
#include <QTimer>

QmlBackend::QmlBackend(QObject *parent)
    : QObject(parent)
{
    connect(m_kernel.model(), &SnippetModel::loaded, this, &QmlBackend::loaded);
    connect(m_kernel.filterModel(), &SnippetProxyModel::filterHasErrorChanged,
            this, &QmlBackend::filterHasErrorChanged);

    QTimer::singleShot(0, &m_kernel, &Kernel::load);
}

QAbstractItemModel *QmlBackend::model() const
{
    return m_kernel.topLevelModel();
}

QString QmlBackend::filterText() const
{
    return m_filterText;
}

void QmlBackend::setFilterText(const QString &text)
{
    if (text == m_filterText)
        return;

    m_filterText = text;
    m_kernel.filterModel()->setFilterText(text);
    emit filterTextChanged(text);
}

bool QmlBackend::isDeepSearch() const
{
    return m_kernel.filterModel()->isDeepSearch();
}

void QmlBackend::setDeepSearch(bool deep)
{
    if (deep == isDeepSearch())
        return;

    m_kernel.filterModel()->setIsDeepSearch(deep);
    emit deepSearchChanged(deep);
}

bool QmlBackend::filterHasError() const
{
    return m_kernel.filterModel()->filterHasError();
}

bool QmlBackend::hasSelection() const
{
    return m_currentIndex.isValid();
}

bool QmlBackend::currentIsFolder() const
{
    return m_currentIndex.isValid() && m_currentIndex.data(SnippetModel::IsFolderRole).toBool();
}

Snippet *QmlBackend::currentSnippet() const
{
    if (!m_currentIndex.isValid() || currentIsFolder())
        return nullptr;

    return m_currentIndex.data(SnippetModel::SnippetRole).value<Snippet *>();
}

QString QmlBackend::currentTitle() const
{
    if (currentIsFolder())
        return m_currentIndex.data(SnippetModel::FolderNameRole).toString();

    Snippet *snippet = currentSnippet();
    return snippet ? snippet->title() : QString();
}

QString QmlBackend::currentTags() const
{
    Snippet *snippet = currentSnippet();
    return snippet ? snippet->tagsString() : QString();
}

QString QmlBackend::currentContents() const
{
    Snippet *snippet = currentSnippet();
    return snippet ? snippet->contents() : QString();
}

void QmlBackend::setCurrentIndex(const QModelIndex &proxyIndex)
{
    if (m_currentIndex == QPersistentModelIndex(proxyIndex))
        return;

    m_currentIndex = proxyIndex;
    emit currentChanged();
}

void QmlBackend::setCurrentTitle(const QString &title)
{
    if (!m_currentIndex.isValid())
        return;

    m_kernel.model()->setData(m_kernel.mapToSource(m_currentIndex), title, Qt::EditRole);
}

void QmlBackend::setCurrentTags(const QString &text)
{
    if (Snippet *snippet = currentSnippet())
        snippet->setTags(text);
}

void QmlBackend::setCurrentContents(const QString &text)
{
    if (Snippet *snippet = currentSnippet())
        snippet->setContents(text);
}

QModelIndex QmlBackend::parentForNewItem() const
{
    if (!m_currentIndex.isValid())
        return {};

    return currentIsFolder() ? QModelIndex(m_currentIndex) : m_currentIndex.parent();
}

QModelIndex QmlBackend::createSnippet()
{
    QModelIndex newSourceIndex = m_kernel.model()->addSnippet(m_kernel.mapToSource(parentForNewItem()));
    return m_kernel.mapFromSource(newSourceIndex);
}

QModelIndex QmlBackend::createFolder(const QString &name)
{
    QStandardItem *newItem = m_kernel.model()->createFolder(name, m_kernel.mapToSource(parentForNewItem()));
    return newItem ? m_kernel.mapFromSource(newItem->index()) : QModelIndex();
}

void QmlBackend::deleteCurrent()
{
    if (!m_currentIndex.isValid())
        return;

    m_kernel.model()->removeSnippet(m_kernel.mapToSource(m_currentIndex));
    setCurrentIndex(QModelIndex());
}

void QmlBackend::reload()
{
    m_kernel.model()->load();
}
