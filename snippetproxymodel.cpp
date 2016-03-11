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

#include "snippetproxymodel.h"
#include "snippetmodel.h"

#include <QDebug>

SnippetProxyModel::SnippetProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SnippetProxyModel::rowsInserted, this, &SnippetProxyModel::countChanged);
    connect(this, &SnippetProxyModel::rowsRemoved, this, &SnippetProxyModel::countChanged);
    connect(this, &SnippetProxyModel::modelReset, this, &SnippetProxyModel::countChanged);
    connect(this, &SnippetProxyModel::layoutChanged, this, &SnippetProxyModel::countChanged);
}

bool SnippetProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!sourceModel() || source_row < 0 || source_row >= sourceModel()->rowCount(source_parent))
        return false;

    if (m_text.isEmpty())
        return true;

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    const bool isFolder = idx.data(SnippetModel::IsFolderRole).toBool();
    const QString title = idx.data(Qt::DisplayRole).toString();
    if (title.contains(m_text, Qt::CaseInsensitive))
        return true;

    if (source_parent.isValid()) {
        const QString absolutePath = source_parent.data(SnippetModel::AbsolutePathRole).toString();
        if (absolutePath.contains(m_text, Qt::CaseInsensitive))
            return true;
    }

    if (isFolder) {
        const int numChildren = sourceModel()->rowCount(idx);
        for (int i = 0; i < numChildren; ++i) {
            if (filterAcceptsRow(i, idx))
                return true;
        }
    } else {
        Snippet *snippet = idx.data(SnippetModel::SnippetRole).value<Snippet*>();
        if (title == SnippetModel::emptySnippetTitle())
            return true;

        foreach (const QString &tag, snippet->tags()) {
            if (tag.contains(m_text, Qt::CaseInsensitive))
                return true;
        }

        if (m_deepSearch) {
            if (snippet->contents().contains(m_text, Qt::CaseInsensitive))
                return true;
        }
    }

    return false;
}

bool SnippetProxyModel::isDeepSearch() const
{
    return m_deepSearch;
}

void SnippetProxyModel::setIsDeepSearch(bool is)
{
    if (is != m_deepSearch) {
        m_deepSearch = is;
        invalidateFilter();
    }
}

void SnippetProxyModel::setFilterText(QString text)
{
    text = text.toLower();
    if (text != m_text) {
        m_text = text;
        invalidateFilter();
    }
}
