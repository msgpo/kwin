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
    const std::chrono::milliseconds delta(time);

    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        (*animationIt).timeLine.update(delta);
        ++animationIt;
    }

    effects->prePaintScreen(data, time);
}

void SlidingNotificationsEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    if (m_animations.contains(w)) {
        data.setTransformed();
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_DELETE);
    }

    effects->prePaintWindow(w, data, time);
}

void SlidingNotificationsEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt == m_animations.constEnd()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    const QRectF screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRectF windowRect = w->expandedGeometry();
    const qreal t = (*animationIt).timeLine.value();

    switch ((*animationIt).screenEdge) {
    case ScreenEdge::Top:
    case ScreenEdge::Bottom:
        break;

    case ScreenEdge::Left: {
        const QRect clipRect(screenRect.left(), windowRect.top(),
            windowRect.right() - screenRect.left(), windowRect.height());
        data.translate(-interpolate(clipRect.width(), 0, t), 0);
        region = QRegion(clipRect);
        break;
    }

    case ScreenEdge::Right: {
        const QRect clipRect(windowRect.left(), windowRect.top(),
            screenRect.right() - windowRect.left(), windowRect.height());
        data.translate(interpolate(clipRect.width(), 0, t), 0);
        region = QRegion(clipRect);
        break;
    }

    default:
        Q_UNREACHABLE();
        break;
    }

    effects->paintWindow(w, mask, region, data);
}

void SlidingNotificationsEffect::postPaintScreen()
{
    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        if ((*animationIt).timeLine.done()) {
            EffectWindow *w = animationIt.key();
            if (w->isDeleted()) {
                w->unrefWindow();
            } else {
                w->setData(WindowForceBackgroundContrastRole, QVariant());
                w->setData(WindowForceBlurRole, QVariant());
            }
            animationIt = m_animations.erase(animationIt);
        } else {
            ++animationIt;
        }
    }

    // TODO: Don't do fullscreen repaints.
    effects->addRepaintFull();

    effects->postPaintScreen();
}

bool SlidingNotificationsEffect::isActive() const
{
    return !m_animations.isEmpty();
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

    Q_UNUSED(screenRect)
    Q_UNUSED(windowRect)

    Animation &animation = m_animations[w];
    animation.screenEdge = ScreenEdge::Right; // TODO: Figure out screen edge.
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Forward);
    animation.timeLine.setDuration(m_duration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

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

    Q_UNUSED(screenRect)
    Q_UNUSED(windowRect)

    Animation &animation = m_animations[w];
    animation.screenEdge = ScreenEdge::Right; // TODO: Figure out screen edge.
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Backward);
    animation.timeLine.setDuration(m_duration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    w->refWindow();

    w->setData(WindowClosedGrabRole, QVariant::fromValue(static_cast<void *>(this)));
    w->setData(WindowForceBackgroundContrastRole, QVariant(true));
    w->setData(WindowForceBlurRole, QVariant(true));

    w->addRepaintFull();
}

void SlidingNotificationsEffect::slotWindowDeleted(EffectWindow *w)
{
    m_animations.remove(w);
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
