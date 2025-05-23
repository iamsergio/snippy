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

#ifndef SNIPPY_SNIPPET_PROXY_MODEL_H
#define SNIPPY_SNIPPET_PROXY_MODEL_H

#include <QSortFilterProxyModel>
#include <QJSEngine>

class SnippetProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SnippetProxyModel(QObject *parent = nullptr);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    bool isDeepSearch() const;
    void setIsDeepSearch(bool);
    void setFilterText(QString);
    bool filterHasError() const;

    QStringList searchTokens() const;

Q_SIGNALS:
    void filterTextChanged(const QString &text);
    void countChanged();
    void filterHasErrorChanged(bool);

private:
    void verifyExpressionValidity();
    void setFilterHasError(bool);
    // Checks if we accept the row, given the line edit filter text, like: "foo & bar"
    bool accepts(const QModelIndex &idx) const;

    // Checks if we accept the row, given a single search token, like "foo".
    bool accepts(const QString &token, const QModelIndex &idx) const;

    bool m_deepSearch = false;
    QString m_text;
    QStringList m_searchTokens;
    mutable QJSEngine m_jsEngine;
    bool m_filterHasError = false;
};

#endif
