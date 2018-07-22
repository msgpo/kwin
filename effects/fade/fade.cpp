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
#include "fade.h"

namespace KWin
{

static const QSet<QString> s_blacklist {
    QStringLiteral("ksmserver ksmserver"),
    QStringLiteral("ksplashqml ksplashqml"),
    QStringLiteral("ksplashsimple ksplashsimple"),
    QStringLiteral("ksplashx ksplashx")
};

FadeEffect::FadeEffect()
{
    reconfigure(ReconfigureAll);

    connect(effects, &EffectsHandler::windowAdded, this, &FadeEffect::fadeIn);
    connect(effects, &EffectsHandler::windowClosed, this, &FadeEffect::fadeOut);
    connect(effects, &EffectsHandler::windowShown, this, &FadeEffect::fadeIn);
    connect(effects, &EffectsHandler::windowHidden, this, &FadeEffect::fadeOut);
    connect(effects, &EffectsHandler::windowDeleted, this, &FadeEffect::windowDeleted);
    connect(effects, &EffectsHandler::windowDataChanged, this, &FadeEffect::windowDataChanged);
    connect(effects, &EffectsHandler::windowMinimized, this, &FadeEffect::windowMinimized);
}

FadeEffect::~FadeEffect()
{
}

void FadeEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    m_fadeInDuration = std::chrono::milliseconds(
        static_cast<int>(animationTime(150)));
    m_fadeOutDuration = std::chrono::milliseconds(
        static_cast<int>(animationTime(150 * 4)));
}

void FadeEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    const std::chrono::milliseconds delta(time);

    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        (*animationIt).update(delta);
        ++animationIt;
    }

    effects->prePaintScreen(data, time);
}

void FadeEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    if (m_animations.contains(w)) {
        data.setTranslucent();
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_DELETE);
    }

    effects->prePaintWindow(w, data, time);
}

void FadeEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt != m_animations.constEnd()) {
        data.multiplyOpacity((*animationIt).timeLine.value());
    }

    effects->paintWindow(w, mask, region, data);
}

void FadeEffect::postPaintScreen()
{
    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        EffectWindow *w = animationIt.key();
        w->addRepaintFull();
        if ((*animationIt).timeLine.done()) {
            if (w->isDeleted()) {
                w->unrefWindow();
            }
            animationIt = m_animations.erase(animationIt);
        } else {
            ++animationIt;
        }
    }

    effects->postPaintScreen();
}

void FadeEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool FadeEffect::supported()
{
    return effects->animationsSupported();
}

static bool isFadeWindow(const EffectWindow *w)
{
    if (s_blacklist.contains(w->windowClass())) {
        return false;
    }

    return true;
}

void FadeEffect::fadeIn(EffectWindow *w)
{
    // TODO: describe why.
    const bool dontFade = m_tempFadeInBlacklist.remove(w);
    if (dontFade) {
        return;
    }

    if (effects->activeFullScreenEffect()) {
        return;
    }

    if (!isFadeWindow()) {
        return;
    }

    if (!w->isVisible()) {
        return;
    }

    const auto *addedGrab = w->data(WindowAddedGrabRole).value<const void*>();
    if (addedGrab != nullptr) {
        return;
    }

    Animation &animation = m_animations[w];
    animation.kind = AnimationKind::In;
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Forward);
    animation.timeLine.setDuration(m_fadeInDuration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    // TODO: describe why this effect is not grabbing this window.

    w->addRepaintFull();
}

void FadeEffect::fadeOut(EffectWindow *w)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    if (!isFadeWindow(w)) {
        return;
    }

    if (!w->isVisible()) {
        return;
    }

    const auto *closedGrab = w->data(WindowClosedGrabRole).value<const void*>();
    if (closedGrab != nullptr) {
        return;
    }

    if (w->isDeleted()) {
        w->refWindow();
    }

    Animation &animation = m_animations[w];
    animation.kind = AnimationKind::Out;
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Backward);
    animation.timeLine.setDuration(m_fadeOutDuration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    // TODO: describe why this effect is not grabbing the window.

    w->addRepaintFull();
}

void FadeEffect::windowDeleted(EffectWindow *w)
{
    m_animations.remove(w);
    m_tempFadeInBlacklist.remove(w);
}

void FadeEffect::windowDataChanged(EffectWindow *w, int role)
{
    if (role != WindowAddedGrabRole && role != WindowClosedGrabRole) {
        return;
    }

    auto animationIt = m_animations.find(w);
    if (animationIt == m_animations.end()) {
        return;
    }

    if (w->isDeleted() && (*animationIt).kind == AnimationKind::Out) {
        w->unrefWindow();
    }

    m_animations.erase(animationIt);
}

void FadeEffect::windowMinimized(EffectWindow *w)
{
    // TODO: describe why.
    m_tempFadeInBlacklist.add(w);

    // TODO: describe why.
    m_animations.remove(w);
}

} // namespace KWin
