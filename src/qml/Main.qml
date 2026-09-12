// Copyright (c) 2026 Sergio Martins <iamsergio@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QC
import QtQuick.Layouts
import QtQml.Models

import snippy 1.0

QC.ApplicationWindow {
    id: window

    visible: true
    width: 900
    height: 640
    // Below this, the header RowLayout's fixed-width children (buttons, filter field,
    // checkbox) no longer all fit and the excess is silently pushed off-window instead of
    // wrapping or shrinking — a QWidget layout would refuse to shrink past its size hint
    // the same way, so mirror that here instead of teaching the toolbar to wrap.
    minimumWidth: 730
    minimumHeight: 400
    title: "Snippy"

    Component.onCompleted: filterField.forceActiveFocus()

    // SnippetProxyModel only re-evaluates acceptance for branches the view has already
    // queried; TreeView is lazy and never expands on its own, so rows inside a collapsed
    // branch never get re-filtered. Expand everything while a filter is active so every
    // branch gets queried, same as MainWindow::updateFilter() does for the QTreeView.
    function applyFilter() {
        if (filterField.text.length > 0)
            treeView.expandRecursively();
    }

    ItemSelectionModel {
        id: treeSelection
        model: Backend.model
        onCurrentChanged: (current) => Backend.setCurrentIndex(current)
    }

    // tagsField/contentsArea only take their text from Backend.currentTags/currentContents
    // here, on selection change, rather than through a plain property binding: typing into
    // either field breaks a "text: Backend.current..." binding on the first keystroke, which
    // would leave them showing stale content after switching to a different snippet.
    Connections {
        target: Backend
        function onCurrentChanged() {
            tagsField.text = Backend.currentTags;
            contentsArea.text = Backend.currentContents;
        }
    }

    QC.Dialog {
        id: newFolderDialog
        title: "New folder"
        anchors.centerIn: parent
        modal: true
        standardButtons: QC.Dialog.Ok | QC.Dialog.Cancel

        QC.TextField {
            id: newFolderNameField
            width: 240
            placeholderText: "Folder name"
        }

        onOpened: {
            newFolderNameField.text = "";
            newFolderNameField.forceActiveFocus();
        }
        onAccepted: {
            if (newFolderNameField.text.length > 0) {
                const idx = Backend.createFolder(newFolderNameField.text);
                treeView.expandToIndex(idx);
                treeSelection.setCurrentIndex(idx, ItemSelectionModel.ClearAndSelect);
            }
        }
    }

    QC.Dialog {
        id: renameDialog
        title: "Rename"
        anchors.centerIn: parent
        modal: true
        standardButtons: QC.Dialog.Ok | QC.Dialog.Cancel

        QC.TextField {
            id: renameField
            width: 240
        }

        onOpened: {
            renameField.text = Backend.currentTitle;
            renameField.selectAll();
            renameField.forceActiveFocus();
        }
        onAccepted: {
            if (renameField.text.length > 0)
                Backend.setCurrentTitle(renameField.text);
        }
    }

    QC.Dialog {
        id: deleteConfirmDialog
        title: "Delete snippet"
        anchors.centerIn: parent
        modal: true
        standardButtons: QC.Dialog.Yes | QC.Dialog.No

        QC.Label {
            text: "Delete \"%1\"? This cannot be undone.".arg(Backend.currentTitle)
        }

        onAccepted: Backend.deleteCurrent()
    }

    header: QC.ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 4

            QC.ToolButton {
                text: "New Folder"
                onClicked: newFolderDialog.open()
            }
            QC.ToolButton {
                text: "New Snippet"
                onClicked: {
                    const idx = Backend.createSnippet();
                    treeView.expandToIndex(idx);
                    treeSelection.setCurrentIndex(idx, ItemSelectionModel.ClearAndSelect);
                }
            }
            QC.ToolButton {
                text: "Rename"
                enabled: Backend.hasSelection
                onClicked: renameDialog.open()
            }
            QC.ToolButton {
                text: "Delete"
                // SnippetModel::removeSnippet() refuses folders outright (they may not be
                // empty), so leaving this enabled for a folder just clears the selection
                // with no visible effect instead of actually deleting anything.
                enabled: Backend.hasSelection && !Backend.currentIsFolder
                onClicked: deleteConfirmDialog.open()
            }
            QC.ToolButton {
                text: "Reload"
                onClicked: Backend.reload()
            }

            Item {
                Layout.fillWidth: true
            }

            QC.TextField {
                id: filterField
                Layout.preferredWidth: 260
                placeholderText: "Filter (a & b & (!c | d))"
                palette.text: Backend.filterHasError ? "red" : window.palette.text
                onTextChanged: {
                    Backend.filterText = text;
                    applyFilter();
                }
            }
            QC.CheckBox {
                text: "Search in contents"
                onToggled: {
                    Backend.deepSearch = checked;
                    applyFilter();
                }
            }
        }
    }

    QC.SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        QC.Frame {
            QC.SplitView.preferredWidth: 300
            QC.SplitView.fillHeight: true

            TreeView {
                id: treeView
                anchors.fill: parent
                clip: true
                model: Backend.model

                // Without this, the column's width is its widest delegate's implicit
                // (unelided) content width, so one long title pushes the column past the
                // viewport and every row's "elide: Text.ElideRight" never has to do anything.
                columnWidthProvider: function () {
                    return width;
                }
                selectionModel: treeSelection

                delegate: QC.TreeViewDelegate {
                    contentItem: QC.Label {
                        text: model.display
                        font.bold: model.isFolder
                        elide: Text.ElideRight
                    }
                }
            }
        }

        QC.Frame {
            QC.SplitView.fillWidth: true
            QC.SplitView.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                enabled: Backend.hasSelection && !Backend.currentIsFolder

                RowLayout {
                    Layout.fillWidth: true
                    QC.Label {
                        text: "Tags (separated by ;)"
                    }
                    QC.TextField {
                        id: tagsField
                        Layout.fillWidth: true
                        onEditingFinished: Backend.setCurrentTags(text)
                    }
                }

                QC.TextArea {
                    id: contentsArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    wrapMode: TextEdit.NoWrap
                    onEditingFinished: Backend.setCurrentContents(text)
                }
            }
        }
    }

    footer: QC.Label {
        id: statusLabel
        padding: 4

        Connections {
            target: Backend
            function onLoaded(numSnippets, path) {
                statusLabel.text = "Loaded %1 snippets from %2".arg(numSnippets).arg(path);
            }
        }
    }
}
