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
    connect(effects, &EffectsHandler::windowGeometryShapeChanged,
            this, &SlidingNotificationsEffect::slotWindowGeometryShapeChanged);
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
    effects->prePaintWindow(w, data, time);
}

void SlidingNotificationsEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    effects->paintWindow(w, mask, region, data);
}

void SlidingNotificationsEffect::postPaintScreen()
{
    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        EffectWindow *w = animationIt.key();
        // TODO: Repaint.

        if (!(*animationIt).timeLine.done()) {
            ++animationIt;
            continue;
        }

        const Animation &animation = *animationIt;
        switch (animation.kind) {
        case AnimationKind::SlideOut:
            w->unrefWindow();
            break;

        case AnimationKind::SlideIn:
        case AnimationKind::Move:
            w->setData(WindowForceBackgroundContrastRole, QVariant());
            w->setData(WindowForceBlurRole, QVariant());
            break;

        default:
            Q_UNREACHABLE();
            break;
        }

        animationIt = m_animations.erase(animationIt);
    }

    // TODO: Don't do full screen repaints.
    effects->addRepaintFull();

    if (m_animations.isEmpty()) {
        startNextBatchOfAnimations();
    }

    effects->postPaintScreen();
}

bool SlidingNotificationsEffect::isActive() const
{
    return !m_animations.isEmpty() || !m_queuedAnimations.isEmpty();
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

    if (!isSlideableWindow(w)) {
        return;
    }

    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->expandedGeometry();

    Animation animation;
    animation.kind = AnimationKind::SlideIn;
    animation.screenEdge = inferSlideScreenEdge(w);
    animation.timeLine.setDirection(TimeLine::Forward);
    animation.timeLine.setDuration(m_duration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    animation.fromGeometry = windowRect;
    animation.toGeometry = windowRect;

    switch (animation.screenEdge) {
    case ScreenEdge::Left:
        animation.fromGeometry.moveRight(screenRect.left());
        break;

    case ScreenEdge::Right:
        animation.fromGeometry.moveLeft(screenRect.right());
        break;

    case ScreenEdge::Top:
    case ScreenEdge::Bottom:
    default:
        Q_UNREACHABLE();
        break;
    }

    // TODO: If m_queuedAnimations is not empty and head has the same
    // kind as this one, put the new animation right in m_animations.
    if (m_queuedAnimations.isEmpty()) {
        m_queuedAnimations.enqueue({w, animation});
        m_animations.insert(w, animation);
    } else {
        m_queuedAnimations.enqueue({w, animation});
    }

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

    if (!isSlideableWindow(w)) {
        return;
    }

    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->expandedGeometry();

    Animation animation;
    animation.kind = AnimationKind::SlideOut;
    animation.screenEdge = inferSlideScreenEdge(w);
    animation.timeLine.setDirection(TimeLine::Forward);
    animation.timeLine.setDuration(m_duration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    animation.fromGeometry = windowRect;
    animation.toGeometry = windowRect;

    switch (animation.screenEdge) {
    case ScreenEdge::Left:
        animation.toGeometry.moveRight(screenRect.left());
        break;

    case ScreenEdge::Right:
        animation.toGeometry.moveLeft(screenRect.right());
        break;

    case ScreenEdge::Top:
    case ScreenEdge::Bottom:
    default:
        Q_UNREACHABLE();
        break;
    }

    // TODO: If m_queuedAnimations is not empty and head has the same
    // kind as this one, put the new animation right in m_animations.
    if (m_queuedAnimations.isEmpty()) {
        m_queuedAnimations.enqueue({w, animation});
        m_animations.insert(w, animation);
    } else {
        m_queuedAnimations.enqueue({w, animation});
    }

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

void SlidingNotificationsEffect::slotWindowGeometryShapeChanged(EffectWindow *w, const QRect &old)
{
    if (!isNotificationWindow(w)) {
        return;
    }
    Q_UNUSED(old)
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

bool SlidingNotificationsEffect::isSlideableWindow(const EffectWindow *w) const
{
    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->expandedGeometry();

    const int centerX = screenRect.center().x();
    if (windowRect.left() < centerX && centerX < windowRect.right()) {
        return false;
    }

    return true;
}

SlidingNotificationsEffect::ScreenEdge SlidingNotificationsEffect::inferSlideScreenEdge(const EffectWindow *w) const
{
    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->expandedGeometry();

    if (windowRect.right() < screenRect.center().x()) {
        return ScreenEdge::Left;
    }

    return ScreenEdge::Right;
}

void SlidingNotificationsEffect::startNextBatchOfAnimations()
{
    const AnimationKind currentAnimationKind = m_queuedAnimations.head().animation.kind;
    while (!m_queuedAnimations.isEmpty()
                && m_queuedAnimations.head().animation.kind != currentAnimationKind) {
        m_queuedAnimations.removeFirst();
    }

    if (m_queuedAnimations.isEmpty()) {
        return;
    }

    const AnimationKind nextAnimationKind = m_queuedAnimations.head().animation.kind;
    for (const QueuedAnimation &queuedAnimation : m_queuedAnimations) {
        if (queuedAnimation.animation.kind != nextAnimationKind) {
            break;
        }
        m_animations[queuedAnimation.target] = queuedAnimation.animation;
    }
}

} // namespace KWin
