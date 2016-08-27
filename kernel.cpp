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

#include "kernel.h"

Kernel::Kernel(QObject *parent)
    : QObject(parent)
    , m_model(new SnippetModel(this))
    , m_filterModel(new SnippetProxyModel(this))
    , m_externalEditor(QString::fromUtf8(qgetenv("SNIPPY_EDITOR")))
    , m_externalFileExplorer(QString::fromUtf8(qgetenv("SNIPPY_FILE_EXPLORER")))
{
    m_filterModel->setSourceModel(m_model);
}

SnippetProxyModel *Kernel::filterModel() const
{
    return m_filterModel;
}

SnippetModel *Kernel::model() const
{
    return m_model;
}

void Kernel::load()
{
    m_model->load();
}

QString Kernel::externalEditor() const
{
    return m_externalEditor;
}

QString Kernel::externalFileExplorer() const
{
    return m_externalFileExplorer;
}
