/*
  Copyright (c) 2015-2016 Sergio Martins <iamsergio@gmail.com>

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
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

enum {
    FilterUpdateTimeout = 400 // ms
};

static void runCommand(const QString &command)
{
    auto proc = new QProcess();
    QObject::connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [proc](int ret, QProcess::ExitStatus) {
        if (ret != 0) {
            qWarning() << "Error running" << proc->program() << proc->arguments();
        }
        proc->deleteLater();
    });
    proc->start(command);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_snippet(nullptr)
{
    qApp->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    setupUi(this);
    m_splitter->setSizes({100, 1000});
    m_treeView->setModel(m_kernel.topLevelModel());

    connect(m_kernel.model(), &SnippetModel::loaded,
            [this](int num, const QString &path) { statusBar()->showMessage(QStringLiteral("Loaded %1 snippets from %2").arg(num).arg(path));});

    /*connect(m_kernel.filterModel(), &SnippetProxyModel::countChanged,
    [this] {
        statusBar()->showMessage(QStringLiteral("Showing %1 snippets").arg(m_kernel.filterModel()->rowCount())); // FIXME: Needs a recursive count
    });*/

    connect(m_tagsLineEdit, &QLineEdit::textChanged, this, &MainWindow::saveNewTags);
    connect(m_textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::saveNewContents);
    connect(m_textEdit, &TextEdit::openExternallyRequested,
            this, &MainWindow::openCurrentSnippetInEditor);

    QTimer::singleShot(0, &m_kernel, &Kernel::load);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_actionReload, &QAction::triggered, m_kernel.model(), &SnippetModel::load);
    connect(m_actionQuit, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_actionOpenDataFolder, &QAction::triggered, this, &MainWindow::openDataFolder);

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
    connect(m_deepSearchCB, &QCheckBox::toggled, this, &MainWindow::updateFilter);

    connect(m_kernel.filterModel(), &SnippetProxyModel::filterHasErrorChanged,
            this, &MainWindow::updateFilterBackground);

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
        QModelIndex selectedProxyIndex = selectedIndex();
        QModelIndex selectedIndex = m_kernel.mapToSource(selectedProxyIndex);
        QStandardItem *newItem = m_kernel.model()->createFolder(name, selectedIndex);
        if (newItem) {
            QModelIndex newProxyIndex = m_kernel.mapFromSource(newItem->index());
            m_treeView->expand(selectedProxyIndex);
            m_treeView->expand(newProxyIndex); // If we create a folder that already exists, expand it, since it has children probably
            m_treeView->scrollTo(newProxyIndex, QAbstractItemView::PositionAtCenter);
            m_treeView->selectionModel()->select(newProxyIndex, QItemSelectionModel::ClearAndSelect);
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

    QModelIndex newIndex = m_kernel.model()->addSnippet(m_kernel.mapToSource(parentIndex));

    if (newIndex.isValid()) {
        m_treeView->expand(parentIndex);
        QModelIndex newProxyIndex = m_kernel.mapFromSource(newIndex);
        m_treeView->scrollTo(newProxyIndex);
        m_treeView->selectionModel()->select(newProxyIndex, QItemSelectionModel::ClearAndSelect);
        m_treeView->edit(newProxyIndex);
    }
}

void MainWindow::deleteSnippet()
{
    QModelIndex proxyIndex = selectedIndex();
    m_kernel.model()->removeSnippet(m_kernel.mapToSource(proxyIndex));
}

void MainWindow::scheduleFilter()
{
    m_scheduleFilterTimer.start(FilterUpdateTimeout);
}

void MainWindow::updateFilter()
{
    const QString text = m_filterLineEdit->text();
    const bool hasText = !text.isEmpty();
    auto filterModel = m_kernel.filterModel();
    filterModel->setFilterText(text);
    filterModel->setIsDeepSearch(m_deepSearchCB->isChecked());
    if (hasText)
        m_treeView->expandAll();

    if (hasText && !selectedIndex().isValid()) {
        QModelIndex index = firstSnippet(QModelIndex());
        if (index.isValid()) {
            m_treeView->selectionModel()->select(index, QItemSelectionModel::Select);
        }
    }
}

QModelIndex MainWindow::firstSnippet(const QModelIndex &index) const
{
    if (index.isValid() && !index.data(SnippetModel::IsFolderRole).toBool())
        return index;

    auto model = m_kernel.topLevelModel();
    const int count = model->rowCount(index);
    for (int row = 0; row < count; ++row) {
        QModelIndex candidate = firstSnippet(model->index(row, 0, index));
        if (candidate.isValid())
            return candidate;
    }

    return {};
}

void MainWindow::openCurrentSnippetInEditor()
{
    if (!m_snippet) {
        qWarning() << "No snippet selected";
        return;
    }

    QString editorCommand = m_kernel.externalEditor();
    if (editorCommand.isEmpty()) {
        qWarning() << "No external editor specified.<br>";
        qWarning() << "Please set the SNIPPY_EDITOR env variable. For example to"
                   << "<b>\"kate %1\"</b>";
        return;
    }

    const QString filename = m_snippet->absolutePath();

    QString fullCommand = editorCommand;
    if (editorCommand.contains(QLatin1String("%1"))) {
        fullCommand = fullCommand.arg(filename);
    } else {
        fullCommand += " " + filename;
    }

    runCommand(fullCommand);
}

void MainWindow::openFileExplorer(QString path)
{
    QFileInfo pathInfo(path);
    path = pathInfo.absoluteFilePath(); // Cleanup path, remove double //

    QString fileExplorerCommand = m_kernel.externalFileExplorer();
    if (fileExplorerCommand.isEmpty()) {
        qWarning() << "Please set the SNIPPY_FILE_EXPLORER env variable. For example to"
                   << "<b>\"dolphin %1\"</b>";
        return;
    }

    qDebug() << "Opening" << path << "...";
    if (fileExplorerCommand.contains(QLatin1String("%1"))) {
        fileExplorerCommand = fileExplorerCommand.arg(path);
    } else {
        fileExplorerCommand += ' ' + path;
    }

    runCommand(fileExplorerCommand);
}

void MainWindow::openDataFolder()
{
    openFileExplorer(m_kernel.model()->snippetDataFolder());
}

void MainWindow::updateFilterBackground(bool isError)
{
    if (isError)
        m_filterLineEdit->setStyleSheet("QLineEdit { color: red; }");
    else
        m_filterLineEdit->setStyleSheet(QString());
}

QModelIndex MainWindow::selectedIndex() const
{
    const QModelIndexList indexes = m_treeView->selectionModel()->selectedIndexes();
    return indexes.isEmpty() ? QModelIndex() : indexes.first();
}
