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

#include "snippetproxymodel.h"
#include "snippetmodel.h"

#include <QDebug>
#include <QRegularExpression>

SnippetProxyModel::SnippetProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SnippetProxyModel::rowsInserted, this, &SnippetProxyModel::countChanged);
    connect(this, &SnippetProxyModel::rowsRemoved, this, &SnippetProxyModel::countChanged);
    connect(this, &SnippetProxyModel::modelReset, this, &SnippetProxyModel::countChanged);
    connect(this, &SnippetProxyModel::layoutChanged, this, &SnippetProxyModel::countChanged);

    connect(this, &SnippetProxyModel::rowsRemoved, [this](const QModelIndex &parent, int, int) {
        if (parent.isValid()) {
            // The model that sits on top of this one filters out parents with no children
            // So trigger it's filterAcceptsRow() to run
            emit dataChanged(parent, parent);
        }
    });

    connect(this, &SnippetProxyModel::rowsInserted, [this](const QModelIndex &parent, int, int) {
        if (parent.isValid()) {
            // The model that sits on top of this one filters out parents with no children
            // So trigger it's filterAcceptsRow() to run
            emit dataChanged(parent, parent);
        }
    });
}

bool SnippetProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!sourceModel() || source_row < 0 || source_row >= sourceModel()->rowCount(source_parent))
        return false;

    if (m_filterHasError)
        return false;

    return accepts(sourceModel()->index(source_row, 0, source_parent));
}

static QStringList tokensFromString(const QString &str)
{
    // Example:
    // input: m_text is "a & b & (!c | d)"
    // output: { "a", "b", "c", "d" }

    static QRegularExpression tokenSeparatorsRegex(R"(&|\||\(|\)|!| )");
    return str.split(tokenSeparatorsRegex);
}

static QString removeSpecialChars(const QString &token)
{
    QString result = token;

    if (!token.isEmpty() && token[0] == QLatin1Char(':'))
        result.remove(0, 1);

    return result;
}

static QString normalizeTextForJS(const QString &token)
{
    QString result = token;
    // TODO: use regexp instead
    result.replace(".", QString());
    result.replace(":", QString());
    result.replace("+", QString());
    result.replace("-", QString());
    return removeSpecialChars(result);
}

bool SnippetProxyModel::accepts(const QModelIndex &idx) const
{
    if (m_text.isEmpty())
        return true;

    QVector<bool> results;
    results.reserve(m_searchTokens.size());
    for (const QString &token : m_searchTokens) {
        m_jsEngine.globalObject().setProperty(normalizeTextForJS(token), accepts(token, idx));
    }

    QJSValue result = m_jsEngine.evaluate(normalizeTextForJS(m_text));

    if (result.isError()) {
        qWarning() << "Filter has errors" << m_text << result.toString();
        return false;
    } else {
        return result.toBool();
    }
}

bool SnippetProxyModel::accepts(const QString &searchToken, const QModelIndex &idx) const
{
    if (searchToken.isEmpty())
        return true;

    QString filterText = removeSpecialChars(searchToken);
    const bool foldersOnly = searchToken.startsWith(':');

    filterText.replace(QRegularExpression("\\/*$"), QString()); // Remove trailling slash
    filterText.replace(QRegularExpression("\\\\*$"), QString()); // Remove trailling back-slash

    QModelIndex parent = idx.parent();
    const bool isFolder = idx.data(SnippetModel::IsFolderRole).toBool();

    if (parent.isValid()) {
        const QString absolutePath = parent.data(SnippetModel::RelativePathRole).toString();
        if (absolutePath.contains(filterText, Qt::CaseInsensitive))
            return true;
    }

    const QString title = idx.data(Qt::DisplayRole).toString();

    if (foldersOnly && (!isFolder && title != SnippetModel::emptySnippetTitle()))
        return false;

    if (title.contains(filterText, Qt::CaseInsensitive))
        return true;

    if (isFolder) {
        const int numChildren = sourceModel()->rowCount(idx);
        for (int i = 0; i < numChildren; ++i) {
            if (accepts(searchToken, sourceModel()->index(i, 0, idx)))
                return true;
        }
    } else {
        Snippet *snippet = idx.data(SnippetModel::SnippetRole).value<Snippet *>();
        if (title == SnippetModel::emptySnippetTitle())
            return true;

        foreach (const QString &tag, snippet->tags()) {
            if (tag.contains(filterText, Qt::CaseInsensitive))
                return true;
        }

        if (m_deepSearch) {
            if (snippet->contents().contains(filterText, Qt::CaseInsensitive))
                return true;
        }
    }

    return false;
}

bool SnippetProxyModel::isDeepSearch() const
{
    return m_deepSearch;
}

void SnippetProxyModel::setIsDeepSearch(bool is)
{
    if (is != m_deepSearch) {
        m_deepSearch = is;
        invalidateFilter();
    }
}

void SnippetProxyModel::verifyExpressionValidity()
{
    if (m_text.isEmpty()) {
        setFilterHasError(false);
        return;
    }

    // Do an evaluation with dummy values just to verify syntax,
    // so that UI can make the line edit red

    QJSEngine engine;
    bool canEvaluate = false;
    for (const QString &token : m_searchTokens) {
        QString normalizedToken = normalizeTextForJS(token);
        if (!normalizedToken.isEmpty()) {
            engine.globalObject().setProperty(normalizedToken, true);
            canEvaluate = true;
        }
    }

    if (canEvaluate) {
        QJSValue result = engine.evaluate(normalizeTextForJS(m_text));
        setFilterHasError(result.isError());
    } else {
        setFilterHasError(true);
    }
}

void SnippetProxyModel::setFilterText(QString text)
{
    text = text.toLower();
    if (text != m_text) {
        m_text = text;
        m_searchTokens = tokensFromString(m_text);

        for (const QString &token : m_searchTokens)
            m_jsEngine.globalObject().deleteProperty(token);

        verifyExpressionValidity();

        if (!m_filterHasError) {
            invalidateFilter();
            filterTextChanged(m_text);
        }
    }
}

bool SnippetProxyModel::filterHasError() const
{
    return m_filterHasError;
}

QStringList SnippetProxyModel::searchTokens() const
{
    return m_searchTokens;
}

void SnippetProxyModel::setFilterHasError(bool has)
{
    if (has != m_filterHasError) {
        m_filterHasError = has;
        emit filterHasErrorChanged(m_filterHasError);
    }
}
