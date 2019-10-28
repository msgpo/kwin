/*
 * Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "scriptsfilterproxymodel.h"
#include "scriptsmodel.h"

namespace KWin
{

ScriptsFilterProxyModel::ScriptsFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

ScriptsFilterProxyModel::~ScriptsFilterProxyModel()
{
}

QString ScriptsFilterProxyModel::query() const
{
    return m_query;
}

void ScriptsFilterProxyModel::setQuery(const QString &query)
{
    if (m_query != query) {
        m_query = query;
        emit queryChanged();
        invalidateFilter();
    }
}

bool ScriptsFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);

    if (!m_query.isEmpty()) {
        const bool matches = idx.data(ScriptsModel::NameRole).toString().contains(m_query, Qt::CaseInsensitive) ||
            idx.data(ScriptsModel::DescriptionRole).toString().contains(m_query, Qt::CaseInsensitive);
        if (!matches) {
            return false;
        }
    }

    return true;
}

}
