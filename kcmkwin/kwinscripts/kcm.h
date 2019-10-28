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

#pragma once

#include <KQuickAddons/ConfigModule>

#include <QAbstractListModel>

namespace KWin
{

class ScriptsModel;

class ScriptsKCM : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QAbstractListModel *scriptsModel READ scriptsModel CONSTANT)

public:
    explicit ScriptsKCM(QObject *parent = nullptr, const QVariantList &list = {});
    ~ScriptsKCM() override;

    QAbstractListModel *scriptsModel() const;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    ScriptsModel *m_model;

    Q_DISABLE_COPY(ScriptsKCM)
};

}
