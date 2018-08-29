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

// own
#include "slidingnotifications.h"

// KConfigSkeleton
#include "slidingnotificationsconfig.h"

namespace KWin
{

static const int IsNotificationRole = 0x22a982d4;

SlidingNotificationsEffect::SlidingNotificationsEffect()
{
    initConfig<SlidingNotificationsConfig>();
    reconfigure(ReconfigureAll);

    connect(effects, &EffectsHandler::windowAdded,
            this, &SlidingNotificationsEffect::slotWindowAdded);
    connect(effects, &EffectsHandler::windowClosed,
            this, &SlidingNotificationsEffect::slotWindowClosed);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &SlidingNotificationsEffect::slotWindowDeleted);
}

void SlidingNotificationsEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    SlidingNotificationsConfig::self()->read();
    m_duration = std::chrono::milliseconds(animationTime<SlidingNotificationsConfig>(250));
}

void SlidingNotificationsEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    effects->prePaintScreen(data, time);
}

void SlidingNotificationsEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    effects->prePaintWindow(w, data, time);
}

void SlidingNotificationsEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    effects->paintWindow(w, mask, region, data);
}

void SlidingNotificationsEffect::postPaintScreen()
{
    effects->postPaintScreen();
}

bool SlidingNotificationsEffect::isActive() const
{
    return false;
}

bool SlidingNotificationsEffect::supported()
{
    return effects->animationsSupported();
}

void SlidingNotificationsEffect::slotWindowAdded(EffectWindow *w)
{
    if (!isNotificationWindow(w)) {
        return;
    }

    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->expandedGeometry();

    // TODO: Setup animation.

    w->setData(IsNotificationRole, QVariant(true));
    w->setData(WindowAddedGrabRole, QVariant::fromValue(static_cast<void *>(this)));
    w->setData(WindowForceBackgroundContrastRole, QVariant(true));
    w->setData(WindowForceBlurRole, QVariant(true));

    w->addRepaintFull();
}

void SlidingNotificationsEffect::slotWindowClosed(EffectWindow *w)
{
    if (!isNotificationWindow(w)) {
        return;
    }

    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->expandedGeometry();

    // TODO: Setup animation.

    w->refWindow();

    w->setData(WindowClosedGrabRole, QVariant::fromValue(static_cast<void *>(this)));
    w->setData(WindowForceBackgroundContrastRole, QVariant(true));
    w->setData(WindowForceBlurRole, QVariant(true));

    w->addRepaintFull();
}

void SlidingNotificationsEffect::slotWindowDeleted(EffectWindow *w)
{
    Q_UNUSED(w)
}

bool SlidingNotificationsEffect::isNotificationWindow(const EffectWindow *w) const
{
    if (w->isNotification()) {
        return true;
    }

    if (w->data(IsNotificationRole).value<bool>()) {
        return true;
    }

    return false;
}

} // namespace KWin
