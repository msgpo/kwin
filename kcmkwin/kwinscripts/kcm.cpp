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

#include "kcm.h"
#include "scriptsfilterproxymodel.h"
#include "scriptsmodel.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(ScriptsKCMFactory,
                           "kcm_kwin_scripts.json",
                           registerPlugin<KWin::ScriptsKCM>();)

namespace KWin
{

ScriptsKCM::ScriptsKCM(QObject *parent, const QVariantList &list)
    : KQuickAddons::ConfigModule(parent, list)
    , m_model(new ScriptsModel(this))
{
    qmlRegisterType<ScriptsFilterProxyModel>("org.kde.private.kcms.kwin.scripts", 1, 0, "ScriptsFilterProxyModel");

    auto about = new KAboutData(QStringLiteral("kcm_kwin_scripts"),
                                i18n("KWin Scripts"),
                                QStringLiteral("2.0"),
                                QString(),
                                KAboutLicense::GPL);
    about->addAuthor(i18n("Vlad Zahorodnii"), QString(), QStringLiteral("vladzzag@gmail.com"));
    setAboutData(about);

    setButtons(Apply | Default);
}

ScriptsKCM::~ScriptsKCM()
{
}

QAbstractListModel *ScriptsKCM::scriptsModel() const
{
    return m_model;
}

void ScriptsKCM::load()
{
    m_model->load();
}

void ScriptsKCM::save()
{
    m_model->save();
}

void ScriptsKCM::defaults()
{
    m_model->defaults();
}

}

#include "kcm.moc"
