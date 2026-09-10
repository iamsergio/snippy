# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Snippy is a Qt Widgets desktop app for browsing and editing a tree of text snippets stored as
`.snip` files on disk. Qt 6 only (developed against 6.10).

## Build / run

```bash
cmake --preset dev      && cmake --build build-dev        # Debug
cmake --preset release  && cmake --build build-release    # RelWithDebInfo
./install.sh                                              # release build + copy over `which snippy`
```

Presets use Ninja and put the build tree in `build-<presetName>/`. `snippy.pro` is a stale qmake
leftover (missing `removeemptyfoldersproxymodel.cpp`); CMake is the real build system.

Runtime configuration is entirely through environment variables:

| Variable | Meaning |
| --- | --- |
| `SNIPPY_FOLDER` | Root of the snippet tree. Falls back to `QStandardPaths::AppDataLocation`. |
| `SNIPPY_EDITOR` | External editor command, e.g. `kate %1`. `%1` is the file path; appended if absent. |
| `SNIPPY_FILE_EXPLORER` | Same convention, e.g. `dolphin %1`. |

Any non-option arguments are joined with spaces and used as the initial filter text
(`snippy foo & bar`). `--quit-after-loading` exits once the tree is loaded, for benchmarking.

## Tests

There are no C++ tests. The only test suite lives in the `rust/` subproject:

```bash
cd rust && cargo test
cd rust && cargo test tst_read_snippet    # single test
```

`rust/` is a standalone experimental reimplementation of the snippet *loader* (`SnippetFolder` /
`Snippet` in `src/lib.rs`). It is not linked into the Qt app and is not built by CMake. Its
fixtures in `rust/test_data/` are the clearest specification of the on-disk format.

## Architecture

### Storage format

The directory tree *is* the model tree. Each snippet is one `.snip` file:

```
line 1     title
line 2     tags, ';'-separated
line 3+    contents
```

`Snippet` autosaves 2 seconds after any setter via a single-shot `QTimer` (`scheduleSave()`) —
there is no explicit save action, so UI edits reach disk on their own. Creating a snippet writes a
UUID-named file immediately. Renaming a folder renames the directory.

### Model stack

`SnippetModel` (a `QStandardItemModel` mirroring the disk tree) → `SnippetProxyModel` (the filter) →
`QTreeView`. `RemoveEmptyFoldersProxyModel` is a third layer that is **currently commented out** in
`kernel.cpp` while still being compiled.

`Kernel` is the only class that knows how deep the stack is. Use `Kernel::topLevelModel()`,
`mapToSource()` and `mapFromSource()` rather than talking to a specific proxy — that is what makes
re-enabling or adding a proxy layer a local change.

Custom roles (`SnippetRole`, `IsFolderRole`, `AbsolutePathRole`, `RelativePathRole`, …) are declared
in `SnippetModel::Role`; `SnippetRole` carries a `Snippet *` through `QVariant`.

### The filter is a JavaScript expression

This is the least obvious part of the codebase. `SnippetProxyModel` does not use
`setFilterRegularExpression`; it overrides `filterAcceptsRow()` and evaluates the filter text as JS:

1. The filter string (e.g. `a & b & (!c | d)`) is split on `& | ( ) !` and space into tokens.
2. Each token is matched against the row (title, tags, parent path — plus contents when deep search
   is on) producing a bool.
3. Those bools are set as global properties on a `QJSEngine`, then the whole filter string is
   `evaluate()`d as a JS expression.

`normalizeTextForJS()` strips `.`, `:`, `+` and `-` so tokens are valid JS identifiers, which means
those characters are silently ignored when searching. A leading `:` on a token restricts it to
folders.

`verifyExpressionValidity()` re-evaluates the expression with dummy `true` values on a throwaway
engine purely so the line edit can be tinted red. While `m_filterHasError` is set,
`filterAcceptsRow()` rejects everything. Filter updates are debounced 400 ms in `MainWindow`.

### UI

`MainWindow` privately inherits `Ui::MainWindow`, so widget members (`m_treeView`,
`m_filterLineEdit`, `m_deepSearchCB`, …) come from `mainwindow.ui` and are not declared in the
header. `SyntaxHighlighter` highlights the current search tokens inside the snippet text.

## Conventions

- Format with the repo `.clang-format` (WebKit-based, `ColumnLimit: 0`, braces on their own line for
  functions and classes only).
- New files carry the existing GPL-2-or-later header with the Qt linking exception.
- `Qt6::Core5Compat` is still linked in `CMakeLists.txt` but nothing uses it any more.
