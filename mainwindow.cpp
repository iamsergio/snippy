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

#include "mainwindow.h"

#include <QTimer>
#include <QItemSelection>
#include <QInputDialog>
#include <QFontDatabase>
#include <QApplication>

enum {
    FilterUpdateTimeout = 400 // ms
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_snippet(nullptr)
{
    qApp->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    setupUi(this);
    m_splitter->setSizes({100, 1000});
    m_treeView->setModel(m_kernel.filterModel());
    connect(m_deepSearchCB, &QCheckBox::toggled, m_kernel.filterModel(), &SnippetProxyModel::setIsDeepSearch);

    connect(m_kernel.model(), &SnippetModel::loaded,
            [this](int num, const QString &path) { statusBar()->showMessage(QStringLiteral("Loaded %1 snippets from %2").arg(num).arg(path));});

    connect(m_kernel.filterModel(), &SnippetProxyModel::countChanged,
    [this] {
        statusBar()->showMessage(QStringLiteral("Showing %1 snippets").arg(m_kernel.filterModel()->rowCount()));
    });

    connect(m_tagsLineEdit, &QLineEdit::textChanged, this, &MainWindow::saveNewTags);
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::saveNewContents);

    QTimer::singleShot(0, &m_kernel, &Kernel::load);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_actionReload, &QAction::triggered, m_kernel.model(), &SnippetModel::load);
    connect(m_actionQuit, &QAction::triggered, qApp, &QApplication::quit);

    m_treeView->setHeaderHidden(true);
    setWindowTitle(tr("Snippy"));

    m_newFolderAction = m_toolBar->addAction(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder), "New Folder");
    connect(m_newFolderAction, &QAction::triggered, this, &MainWindow::createFolder);

    m_newAction = m_toolBar->addAction(QIcon(":/img/list-add.png"), "New Snippet");
    connect(m_newAction, &QAction::triggered, this, &MainWindow::createSnippet);

    m_delAction = m_toolBar->addAction(QIcon(":/img/list-remove.png"), "Remove");
    connect(m_delAction, &QAction::triggered, this, &MainWindow::deleteSnippet);
    m_delAction->setShortcut(QKeySequence(QKeySequence::Delete));

    connect(m_actionExpandAll, &QAction::triggered, m_treeView, &QTreeView::expandAll);

    m_filterLineEdit->setFocus();

    connect(m_filterLineEdit, &QLineEdit::textChanged, this, &MainWindow::scheduleFilter);
    m_scheduleFilterTimer.setSingleShot(true);
    connect(&m_scheduleFilterTimer, &QTimer::timeout, this, &MainWindow::updateFilter);

    // m_treeView->setRootIsDecorated(false); commented out because it's december, the tree should be decorated
    setSnippet(nullptr);
}

void MainWindow::setSnippet(Snippet *snippet)
{
    //if (m_snippet == snippet)
        //return;

    m_snippet = snippet;

    if (snippet) {
        m_textEdit->document()->setPlainText(snippet->contents());
        m_tagsLineEdit->setText(snippet->tagsString());
    } else {
        m_textEdit->document()->setPlainText(QString());
        m_tagsLineEdit->setText(QString());
    }

    m_textEdit->setEnabled(snippet);
    m_tagsLineEdit->setEnabled(snippet);
    m_delAction->setEnabled(snippet);
}

void MainWindow::onSelectionChanged(const QItemSelection &selection, const QItemSelection &/*deselection*/)
{
    const QModelIndexList indexes = selection.indexes();
    if (indexes.isEmpty()) {
        setSnippet(nullptr);
        return;
    }

    const bool isFolder = indexes.first().data(SnippetModel::IsFolderRole).toBool();

    if (isFolder) {
        setSnippet(nullptr);
    } else {
        Snippet *snippet = indexes.first().data(SnippetModel::SnippetRole).value<Snippet*>();
        setSnippet(snippet);
    }
}

void MainWindow::saveNewTags(const QString &text)
{
    if (m_snippet)
        m_snippet->setTags(text);
}

void MainWindow::saveNewContents()
{
    if (m_snippet)
        m_snippet->setContents(m_textEdit->toPlainText());
}

void MainWindow::createFolder()
{
    const QString &name = QInputDialog::getText(this, "Snippy", "Enter folder name");
    if (!name.isEmpty()) {
        QModelIndex index = selectedIndex();
        QStandardItem *newItem = m_kernel.model()->createFolder(name, m_kernel.filterModel()->mapToSource(index));
        if (newItem) {
            m_treeView->expand(index);
            m_treeView->scrollTo(newItem->index());
            m_treeView->selectionModel()->select(newItem->index(), QItemSelectionModel::ClearAndSelect);
        } else {
            statusBar()->showMessage(QStringLiteral("Failed to create folder: %1").arg(name));
        }
    }
}

void MainWindow::createSnippet()
{
    QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
    QModelIndex parentIndex;
    if (!indexes.isEmpty()) {
        QModelIndex index = indexes.at(0);
        if (index.data(SnippetModel::IsFolderRole).toBool()) {
            parentIndex = index;
        } else {
            parentIndex = index.parent();
        }
    }

    QModelIndex newIndex = m_kernel.model()->addSnippet(m_kernel.filterModel()->mapToSource(parentIndex));

    if (newIndex.isValid()) {
        m_treeView->expand(parentIndex);
        QModelIndex newProxyIndex = m_kernel.filterModel()->mapFromSource(newIndex);
        m_treeView->scrollTo(newProxyIndex);
        m_treeView->selectionModel()->select(newProxyIndex, QItemSelectionModel::ClearAndSelect);
        m_treeView->edit(newProxyIndex);
    }
}

void MainWindow::deleteSnippet()
{
    QModelIndex proxyIndex = selectedIndex();
    m_kernel.model()->removeSnippet(m_kernel.filterModel()->mapToSource(proxyIndex));
}

void MainWindow::scheduleFilter()
{
    m_scheduleFilterTimer.start(FilterUpdateTimeout);
}

void MainWindow::updateFilter()
{
    m_kernel.filterModel()->setFilterText(m_filterLineEdit->text());
    if (!m_filterLineEdit->text().isEmpty()) {
        m_treeView->expandAll();
    }
}

QModelIndex MainWindow::selectedIndex() const
{
    const QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
    return indexes.isEmpty() ? QModelIndex() : indexes.first();
}
