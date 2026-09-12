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

#ifndef SNIPPY_FOLDER_ICON_DELEGATE_H
#define SNIPPY_FOLDER_ICON_DELEGATE_H

#include <QStyledItemDelegate>

// Supplies the folder icon at view level, since SnippetModel (shared with the QML frontend)
// no longer depends on QStyle/QApplication to provide it.
class FolderIconDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FolderIconDelegate(QObject *parent = nullptr);

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

#endif
