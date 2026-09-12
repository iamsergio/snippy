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
    // Below this, the header RowLayout's fixed-width children (the toolbar buttons) no
    // longer all fit and the excess is silently pushed off-window instead of wrapping or
    // shrinking — a QWidget layout would refuse to shrink past its size hint the same way,
    // so mirror that here instead of teaching the toolbar to wrap.
    minimumWidth: 730
    minimumHeight: 400
    title: "Snippy"

    property bool markdownPreviewEnabled: false

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
        width: 280
        modal: true
        standardButtons: QC.Dialog.Ok | QC.Dialog.Cancel

        // A bare fixed width here would exceed the dialog's own implicit width (which,
        // absent a fillWidth layout, comes only from the title/buttons) and overflow past
        // its frame instead of being clipped to it.
        QC.TextField {
            id: newFolderNameField
            anchors.left: parent.left
            anchors.right: parent.right
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
        width: 280
        modal: true
        standardButtons: QC.Dialog.Ok | QC.Dialog.Cancel

        QC.TextField {
            id: renameField
            anchors.left: parent.left
            anchors.right: parent.right
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

    menuBar: QC.MenuBar {
        QC.Menu {
            title: "&File"
            QC.MenuItem {
                text: "&Reload"
                onTriggered: Backend.reload()
            }
            QC.MenuItem {
                text: "Expand All"
                onTriggered: treeView.expandRecursively()
            }
            QC.MenuItem {
                text: "&Quit"
                onTriggered: Qt.quit()
            }
        }
        QC.Menu {
            title: "&View"
            QC.MenuItem {
                text: "&Markdown Preview"
                checkable: true
                checked: window.markdownPreviewEnabled
                onToggled: window.markdownPreviewEnabled = checked
            }
        }
        QC.Menu {
            title: "Tools"
            QC.MenuItem {
                text: "Open data folder ..."
                onTriggered: Backend.openDataFolder()
            }
        }
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
            Item {
                Layout.fillWidth: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        QC.SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                        visible: !window.markdownPreviewEnabled
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        wrapMode: TextEdit.NoWrap
                        onEditingFinished: Backend.setCurrentContents(text)
                    }

                    QC.ScrollView {
                        visible: window.markdownPreviewEnabled
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        QC.Label {
                            width: parent.width
                            text: contentsArea.text
                            textFormat: Text.MarkdownText
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            QC.TextField {
                id: filterField
                Layout.fillWidth: true
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
