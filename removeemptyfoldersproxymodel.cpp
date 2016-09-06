/*
  Copyright (c) 2016 Sergio Martins <iamsergio@gmail.com>

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

#include "removeemptyfoldersproxymodel.h"
#include "snippetmodel.h"
#include <QDebug>

RemoveEmptyFoldersProxyModel::RemoveEmptyFoldersProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void RemoveEmptyFoldersProxyModel::setAcceptsEmptyParents(bool accepts)
{
    if (accepts != m_acceptsEmptyParents) {
        m_acceptsEmptyParents = accepts;
        invalidateFilter();
    }
}

bool RemoveEmptyFoldersProxyModel::filterAcceptsRow(int source_row,
                                                    const QModelIndex &source_parent) const
{
    if (m_acceptsEmptyParents)
        return true;

    auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    qDebug() << sourceIndex.model() << sourceIndex.data(SnippetModel::IsFolderRole).toBool();
    if (sourceIndex.data(SnippetModel::IsFolderRole).toBool())
        return sourceModel()->rowCount(sourceIndex) > 0;

    return true;
}
