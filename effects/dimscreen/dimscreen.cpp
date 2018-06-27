/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2008, 2009 Martin Gräßlin <mgraesslin@kde.org>
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
#include "dimscreen.h"

// Qt
#include <QSet>

namespace KWin
{

static const QSet<QString> s_authDialogs {
    QStringLiteral("kdesu kdesu"),
    QStringLiteral("kdesudo kdesudo"),
    QStringLiteral("pinentry pinentry"),
    QStringLiteral("polkit-kde-authentication-agent-1 polkit-kde-authentication-agent-1"),
    QStringLiteral("polkit-kde-manager polkit-kde-manager"),
};

DimScreenEffect::DimScreenEffect()
{
    reconfigure(ReconfigureAll);

    connect(effects, &EffectsHandler::windowActivated,
            this, &DimScreenEffect::windowActivated);
    connect(effects, &EffectsHandler::activeFullScreenEffectChanged,
            this, &DimScreenEffect::activeFullScreenEffectChanged);
}

DimScreenEffect::~DimScreenEffect()
{
}

void DimScreenEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    m_dimTransition.timeLine.setDuration(
        std::chrono::milliseconds(static_cast<int>(animationTime(250))));
}

void DimScreenEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    if (m_dimTransition.active) {
        m_dimTransition.timeLine.update(std::chrono::milliseconds(time));
        m_currentDimStrength = m_dimStrength * m_dimTransition.timeLine.value();
    } else if (effects->activeFullScreenEffect()) {
        m_currentDimStrength = 0.0;
    } else {
        m_currentDimStrength = m_dimStrength;
    }

    effects->prePaintScreen(data, time);
}

void DimScreenEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (canDim(w)) {
        data.multiplyBrightness(1.0 - m_currentDimStrength);
        data.multiplySaturation(1.0 - m_currentDimStrength);
    }

    effects->paintWindow(w, mask, region, data);
}

void DimScreenEffect::postPaintScreen()
{
    if (m_dimTransition.active) {
        if (m_dimTransition.timeLine.done()) {
            m_dimTransition.active = false;
        }
        effects->addRepaintFull();
    }

    effects->postPaintScreen();
}

bool DimScreenEffect::isActive() const
{
    return m_dimTransition.active
        || m_authDialog != nullptr;
}

void DimScreenEffect::windowActivated(EffectWindow *w)
{
    if (!w) {
        return;
    }

    const EffectWindow *prevAuthDialog = m_authDialog;
    m_authDialog = s_authDialogs.contains(w->windowClass()) ? w : nullptr;

    if (effects->activeFullScreenEffect()) {
        return;
    }

    // User switched between authentication dialogs.
    if (m_authDialog != nullptr && prevAuthDialog != nullptr) {
        return;
    }

    // An authentication dialog got focus.
    if (m_authDialog != nullptr && prevAuthDialog == nullptr) {
        if (m_dimTransition.timeLine.done()) {
            m_dimTransition.timeLine.reset();
        }
        m_dimTransition.timeLine.setDirection(TimeLine::Forward);
        m_dimTransition.active = true;
        effects->addRepaintFull();
        return;
    }

    // An authentication dialog lost focus.
    if (m_authDialog == nullptr && prevAuthDialog != nullptr) {
        if (!prevAuthDialog->isOnCurrentDesktop()
                || !prevAuthDialog->isOnCurrentActivity()) {
            m_dimTransition.timeLine.reset();
            m_dimTransition.active = false;
            return;
        }

        if (m_dimTransition.timeLine.done()) {
            m_dimTransition.timeLine.reset();
        }
        m_dimTransition.timeLine.setDirection(TimeLine::Backward);
        m_dimTransition.active = true;
        effects->addRepaintFull();
    }
}

void DimScreenEffect::activeFullScreenEffectChanged()
{
    if (!isActive()) {
        return;
    }

    if (m_dimTransition.timeLine.done()) {
        m_dimTransition.timeLine.reset();
    }
    m_dimTransition.timeLine.setDirection(
        effects->activeFullScreenEffect()
            ? TimeLine::Backward
            : TimeLine::Forward);
    m_dimTransition.active = true;

    effects->addRepaintFull();
}

bool DimScreenEffect::canDim(const EffectWindow *w) const
{
    if (w == m_authDialog) {
        return false;
    }

    return w->isManaged();
}

} // namespace KWin
