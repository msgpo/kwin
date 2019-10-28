/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2018 Vlad Zahorodnii <vladzzag@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "scriptsmodel.h"

namespace KWin
{

ScriptsModel::ScriptsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ScriptsModel::~ScriptsModel()
{
}

QHash<int, QByteArray> ScriptsModel::roleNames() const
{
    QHash<int, QByteArray> additionalRoleNames {
        { NameRole, QByteArrayLiteral("name") },
        { DescriptionRole, QByteArrayLiteral("description") },
        { AuthorNameRole, QByteArrayLiteral("authorName") },
        { AuthorEmailRole, QByteArrayLiteral("authorEmail") },
        { LicenseRole, QByteArrayLiteral("license") },
        { VersionRole, QByteArrayLiteral("version") },
        { EnabledRole, QByteArrayLiteral("enabled") },
        { ConfigurableRole, QByteArrayLiteral("configurable") },
        { ServiceNameRole, QByteArrayLiteral("serviceName") },
        { IconNameRole, QByteArrayLiteral("iconName") },
    };

    return additionalRoleNames.unite(QAbstractListModel::roleNames());
}

int ScriptsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_scripts.count();
}

QVariant ScriptsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const Script script = m_scripts.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return script.name;
    case DescriptionRole:
        return script.description;
    case AuthorNameRole:
        return script.authorName;
    case AuthorEmailRole:
        return script.authorEmail;
    case LicenseRole:
        return script.license;
    case VersionRole:
        return script.version;
    case EnabledRole:
        return script.isEnabled;
    case ConfigurableRole:
        return script.isConfigurable;
    case ServiceNameRole:
        return script.serviceName;
    case IconNameRole:
        return script.iconName;
    default:
        return {};
    }
}

bool ScriptsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return QAbstractItemModel::setData(index, value, role);
    }

    return QAbstractItemModel::setData(index, value, role);
}

void ScriptsModel::load()
{
}

void ScriptsModel::save()
{
}

void ScriptsModel::defaults()
{
}

void ScriptsModel::configure(const QString &scriptId, QWindow *transientParent)
{
    Q_UNUSED(scriptId)
    Q_UNUSED(transientParent)
}

}
