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

#ifndef SNIPPY_KERNEL_H
#define SNIPPY_KERNEL_H

#include "snippetmodel.h"
#include "snippetproxymodel.h"

class Kernel : public QObject
{
    Q_OBJECT
public:
    explicit Kernel(QObject *parent = nullptr);
    SnippetProxyModel *filterModel() const;
    SnippetModel *model() const;
    QString externalEditor() const;
    QString externalFileExplorer() const;
public Q_SLOTS:
    void load();

private:
    SnippetModel *const m_model;
    SnippetProxyModel *const m_filterModel;
    const QString m_externalEditor;
    const QString m_externalFileExplorer;
};

#endif
