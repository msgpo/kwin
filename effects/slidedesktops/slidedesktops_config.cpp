/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>

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

#include "slidedesktops_config.h"
// KConfigSkeleton
#include "slidedesktopsconfig.h"
#include <config-kwin.h>

#include <kwineffects_interface.h>

#include <KAboutData>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(SlideDesktopsEffectConfigFactory,
                           "slidedesktops_config.json",
                           registerPlugin<KWin::SlideDesktopsEffectConfig>();)

namespace KWin
{

SlideDesktopsEffectConfig::SlideDesktopsEffectConfig(QWidget *parent, const QVariantList &args)
    : KCModule(KAboutData::pluginData(QStringLiteral("slidedesktops")), parent, args)
{
    m_ui.setupUi(this);
    SlideDesktopsConfig::instance(KWIN_CONFIG);
    addConfig(SlideDesktopsConfig::self(), this);
    load();
}

SlideDesktopsEffectConfig::~SlideDesktopsEffectConfig()
{
}

void SlideDesktopsEffectConfig::save()
{
    KCModule::save();

    OrgKdeKwinEffectsInterface interface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Effects"),
                                         QDBusConnection::sessionBus());
    interface.reconfigureEffect(QStringLiteral("slidedesktops"));
}

} // namespace KWin

#include "slidedesktops_config.moc"
