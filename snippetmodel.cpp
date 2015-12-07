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

#include "snippetmodel.h"

#include <QStandardPaths>
#include <QStandardItem>
#include <QDir>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QFile>
#include <QUuid>

SnippetModel::SnippetModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_numSnippets(0)
{
}

QVariant SnippetModel::data(const QModelIndex &index, int role) const
{
    QStandardItem *item = itemFromIndex(index);
    if (!item) {
        qWarning() << Q_FUNC_INFO << "Invalid item for index " << index;
        return {};
    }

    if (isFolder(index)) {
        if (role == Qt::DecorationRole) {
            QStyle *style = qApp->style();
            return style->standardIcon(QStyle::SP_DirOpenIcon);
        } else if (role == Qt::DisplayRole) {
            return data(index, FolderNameRole);
        } else if (role == Qt::EditRole) {
            return data(index, FolderNameRole);
        }
    } else {
        Snippet *snip = snippet(index);
        if (role == Qt::DisplayRole) {
            return snip->title();
        } else if (role == Qt::EditRole) {
            return index.data(Qt::DisplayRole);
        }
    }

    return QStandardItemModel::data(index, role);
}

bool SnippetModel::setData(const QModelIndex &index, const QVariant &value, int /*role*/)
{
    QString text = value.toString();
    if (text.isEmpty())
        return false;

    QStandardItem *item = itemFromIndex(index);
    if (isFolder(index)) {
        const QString currentFolderPath = index.data(AbsolutePathRole).toString();
        QDir dir(currentFolderPath);
        dir.cdUp();
        const QString newFolderPath = dir.absoluteFilePath(text);
        bool success = dir.rename(currentFolderPath, newFolderPath);
        if (success) {
            item->setData(text, FolderNameRole);
            item->setData(newFolderPath, AbsolutePathRole);
        }
    } else {
        Snippet *snip = snippet(index);
        snip->setTitle(text);
    }

    return true;
}

bool SnippetModel::isFolder(const QModelIndex &index) const
{
    QStandardItem *item = itemFromIndex(index);
    return item ? item->data(IsFolderRole).toBool() : false;
}

Snippet* SnippetModel::snippet(const QModelIndex &index) const
{
    QStandardItem *item = itemFromIndex(index);
    return item ? item->data(SnippetRole).value<Snippet*>() : nullptr;
}

void SnippetModel::load()
{
    beginResetModel();
    clear();
    m_numSnippets = 0;
    import(QDir(rootPath()), invisibleRootItem());
    endResetModel();

    emit loaded(m_numSnippets, rootPath());
}

void SnippetModel::removeSnippet(const QModelIndex &index)
{
    if (!index.isValid()) {
        qWarning() << Q_FUNC_INFO << "Refusing to delete invalid index";
        return;
    }

    const bool isFolder = index.data(SnippetModel::IsFolderRole).toBool();
    if (isFolder) {
        // TODO: Implement this if folder is empty
        qWarning() << Q_FUNC_INFO << "Refusing to delete folder";
        return;
    }

    Snippet *snippet = index.data(SnippetModel::SnippetRole).value<Snippet*>();
    if (!snippet) {
        qWarning() << Q_FUNC_INFO << "Refusing to delete null snippet";
        return;
    }

    removeRow(index.row(), index.parent());
    if (!QFile::remove(snippet->absolutePath()))
        qWarning() << "Error removing" << snippet->absolutePath();
}

QModelIndex SnippetModel::addSnippet(const QModelIndex &parentIndex)
{
    QString parentFolderPath;
    QStandardItem *parentItem;
    if (parentIndex.isValid()) {
        parentFolderPath = parentIndex.data(AbsolutePathRole).toString();
        parentItem = itemFromIndex(parentIndex);
    } else {
        parentFolderPath = rootPath();
        parentItem = invisibleRootItem();
    }

    if (parentFolderPath.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Could not retrieve parent folder path for" << parentIndex;
        return {};
    }

    QUuid uuid = QUuid::createUuid();
    const QString filename = uuid.toString().replace("{", "").replace("}", "") + ".snip";
    Snippet *snip = new Snippet(this);
    snip->setAbsolutePath(parentFolderPath + "/" + filename);
    snip->setTitle("Empty snippet");
    snip->saveToFile();
    QStandardItem *item = addSnippet(snip, parentItem);
    return indexFromItem(item);
}

bool SnippetModel::createFolder(const QString &name, const QModelIndex &parentIndex)
{
    QString parentFolderPath;
    QStandardItem *parentItem;
    if (parentIndex.isValid()) {
        parentFolderPath = parentIndex.data(AbsolutePathRole).toString();
        parentItem = itemFromIndex(parentIndex);
    } else {
        parentFolderPath = rootPath();
        parentItem = invisibleRootItem();
    }

    if (parentFolderPath.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Could not retrieve parent folder path for" << parentIndex;
        return false;
    }

    QDir dir(parentFolderPath);
    const QString absolutePath = parentFolderPath + "/" + name;
    if (QFile::exists(absolutePath)) {
        qWarning() << "Folder already exists" << absolutePath;
        return false;
    }

    bool success = dir.mkpath(name);

    if (success) {
        addFolder(name, parentFolderPath + "/" + name, parentItem);
    } else {
        qWarning() << "Failed to create folder" << name;
    }

    return success;
}

QStandardItem* SnippetModel::addSnippet(Snippet *snippet, QStandardItem *parentItem)
{
    QStandardItem *fileItem = new QStandardItem();
    fileItem->setData(QVariant::fromValue(snippet), SnippetRole);
    parentItem->appendRow(fileItem);
    m_numSnippets++;

    return fileItem;
}

QStandardItem* SnippetModel::addFolder(const QString &foldername,
                                       const QString &absolutePath, QStandardItem *parentItem)
{
    QStandardItem *folderItem = new QStandardItem(foldername);
    folderItem->setData(true, IsFolderRole);
    folderItem->setData(foldername, FolderNameRole);
    folderItem->setData(absolutePath, AbsolutePathRole);
    parentItem->appendRow(folderItem);
    return folderItem;
}

void SnippetModel::import(QDir dir, QStandardItem *parentItem)
{
    dir.setNameFilters({"*.snip"});
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (const QString &filename, dir.entryList()) {
        Snippet *snippet = new Snippet(this);
        const QString &absoluteFileName = dir.absoluteFilePath(filename);
        snippet->loadFromFile(absoluteFileName);
        if (!snippet->isValid()) {
            qWarning() << Q_FUNC_INFO << "Invalid snippet" << absoluteFileName;
            delete snippet;
            continue;
        }

        addSnippet(snippet, parentItem);
    }

    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    foreach (const QString &foldername, dir.entryList()) {
        const QString absolutePath = dir.absoluteFilePath(foldername);
        QStandardItem *folderItem = addFolder(foldername, absolutePath, parentItem);
        import(QDir(absolutePath), folderItem);
    }
}

QString SnippetModel::rootPath() const
{
    static const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return path;
}
