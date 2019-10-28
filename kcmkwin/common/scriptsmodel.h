/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>

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

#pragma once

#include <kwin_export.h>

#include <QAbstractListModel>
#include <QWindow>

namespace KWin
{

/**
 *
 */
class KWIN_EXPORT ScriptsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ScriptsModel(QObject *parent = nullptr);
    ~ScriptsModel() override;

    enum {
        /**
         *
         */
        NameRole = Qt::UserRole + 1,
        /**
         *
         */
        DescriptionRole,
        /**
         *
         */
        AuthorNameRole,
        /**
         *
         */
        AuthorEmailRole,
        /**
         *
         */
        LicenseRole,
        /**
         *
         */
        VersionRole,
        /**
         *
         */
        EnabledRole,
        /**
         *
         */
        ConfigurableRole,
        /**
         *
         */
        ServiceNameRole,
        /**
         *
         */
        IconNameRole,
    };

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

public Q_SLOTS:
    /**
     *
     */
    void load();

    /**
     *
     */
    void save();

    /**
     *
     */
    void defaults();

    /**
     *
     */
    void configure(const QString &scriptId, QWindow *transientParent = nullptr);

private:
    struct Script
    {
        QString name;
        QString description;
        QString authorName;
        QString authorEmail;
        QString license;
        QString version;
        QString serviceName;
        QString iconName;
        bool isEnabled = false;
        bool isEnabledByDefault = false;
        bool isConfigurable = false;
    };

    QVector<Script> m_scripts;
};

}
