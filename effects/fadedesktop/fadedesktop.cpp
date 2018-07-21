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
#include "fadedesktop.h"

// KConfigSkeleton
#include "fadedesktopconfig.h"

namespace KWin
{

FadeDesktopEffect::FadeDesktopEffect()
{
    initConfig<FadeDesktopConfig>();
    reconfigure(ReconfigureAll);

    connect(effects, static_cast<void (EffectsHandler::*)(int,int,EffectWindow*)>(&EffectsHandler::desktopChanged),
            this, &FadeDesktopEffect::start);
    connect(effects, &EffectsHandler::windowAdded,
            this, &FadeDesktopEffect::windowAdded);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &FadeDesktopEffect::windowDeleted);
}

FadeDesktopEffect::~FadeDesktopEffect()
{
}

void FadeDesktopEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    FadeDesktopConfig::self()->read();

    m_duration = std::chrono::milliseconds(animationTime<FadeDesktopConfig>(250));
}

void FadeDesktopEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    const std::chrono::milliseconds delta(time);

    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        (*animationIt).update(delta);
        ++animationIt;
    }

    effects->prePaintScreen(data, time);
}

void FadeDesktopEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    if (m_animations.contains(w)) {
        data.setTranslucent();
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_DESKTOP);
    }

    effects->prePaintWindow(w, data, time);
}

void FadeDesktopEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt != m_animations.constEnd()) {
        data.multiplyOpacity((*animationIt).value());
    }

    effects->paintWindow(w, mask, region, data);
}

void FadeDesktopEffect::postPaintScreen()
{
    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        EffectWindow *w = animationIt.key();
        w->addRepaintFull();
        if ((*animationIt).done()) {
            animationIt = m_animations.erase(animationIt);
        } else {
            ++animationIt;
        }
    }

    if (m_animations.isEmpty()) {
        stop();
    }

    effects->postPaintScreen();
}

bool FadeDesktopEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool FadeDesktopEffect::supported()
{
    return effects->animationsSupported();
}

void FadeDesktopEffect::start(int oldDesktop, int newDesktop, EffectWindow *with)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this) {
        return;
    }

    const auto windows = effects->stackingOrder();
    for (EffectWindow *w : windows) {
        if (w->isOnAllDesktops()) {
            continue;
        }

        if (w == with) {
            continue;
        }

        // TODO: Handle windows that are on multiple virtual desktops.
        const int desktop = w->desktop();
        if (desktop != oldDesktop && desktop != newDesktop) {
            continue;
        }

        if (w->isMinimized()) {
            continue;
        }

        if (!w->isOnCurrentActivity()) {
            continue;
        }

        TimeLine &timeLine = m_animations[w];
        timeLine.setDuration(m_duration);
        timeLine.setDirection((desktop == newDesktop)
            ? TimeLine::Forward
            : TimeLine::Backward);
        timeLine.setEasingCurve(QEasingCurve::Linear);

        w->addRepaintFull();
    }

    if (m_animations.isEmpty()) {
        return;
    }

    for (EffectWindow *w : windows) {
        w->setData(WindowForceBackgroundContrastRole, QVariant(true));
        w->setData(WindowForceBlurRole, QVariant(true));
    }

    effects->setActiveFullScreenEffect(this);
}

void FadeDesktopEffect::stop()
{
    if (effects->activeFullScreenEffect() != this) {
        return;
    }

    const auto windows = effects->stackingOrder();
    for (EffectWindow *w : windows) {
        w->setData(WindowForceBackgroundContrastRole, QVariant());
        w->setData(WindowForceBlurRole, QVariant());
    }

    effects->setActiveFullScreenEffect(nullptr);
    effects->addRepaintFull();
}

void FadeDesktopEffect::windowAdded(EffectWindow *w)
{
    if (!isActive()) {
        return;
    }

    // NOTE: In case of this effect, that's okay to not fade in the new
    // window because technically it appeared after user switched to
    // another virtual desktop. If the new window is not on the current
    // virtual desktop, it won't be drawn.

    w->setData(WindowForceBackgroundContrastRole, QVariant(true));
    w->setData(WindowForceBlurRole, QVariant(true));
}

void FadeDesktopEffect::windowDeleted(EffectWindow *w)
{
    if (!isActive()) {
        return;
    }

    m_animations.remove(w);

    if (m_animations.isEmpty()) {
        stop();
    }
}

} // namespace KWin
