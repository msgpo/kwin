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
    if (m_animations.contains(w)) {
        data.setTransformed();
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_DELETE);
    } else if (m_interAnimationState.contains(w)) {
        data.setTransformed();
    }

    // TODO: Freeze notifications that have to be moved up while SlideOut is active.

    effects->prePaintWindow(w, data, time);
}

void SlidingNotificationsEffect::paintSlideStage(EffectWindow *w, const Animation &animation, QRegion &region, WindowPaintData &data) const
{
    const qreal t = animation.timeLine.value();

    const QPointF currentPos(
        interpolate(animation.fromGeometry.x(), animation.toGeometry.x(), t),
        interpolate(animation.fromGeometry.y(), animation.toGeometry.y(), t));
    const QPointF diff = currentPos - QPointF(w->geometry().topLeft());

    data.translate(diff.x(), diff.y());

    // TODO: Clip.
}

void SlidingNotificationsEffect::paintMoveStage(EffectWindow *w, const Animation &animation, QRegion &region, WindowPaintData &data) const
{
    Q_UNUSED(region)

    const qreal t = animation.timeLine.value();

    const QPointF currentPos(
        interpolate(animation.fromGeometry.x(), animation.toGeometry.x(), t),
        interpolate(animation.fromGeometry.y(), animation.toGeometry.y(), t));
    const QPointF diff = currentPos - QPointF(w->geometry().topLeft());

    data.translate(diff.x(), diff.y());
}

void SlidingNotificationsEffect::paintAnimation(EffectWindow *w, const Animation &animation, QRegion &region, WindowPaintData &data) const
{
    switch (animation.kind) {
    case AnimationKind::SlideIn:
    case AnimationKind::SlideOut:
        paintSlideStage(w, animation, region, data);
        break;

    case AnimationKind::Move:
        paintMoveStage(w, animation, region, data);
        break;

    default:
        Q_UNREACHABLE();
        break;
    }
}

void SlidingNotificationsEffect::paintInterAnimation(EffectWindow *w, const InterAnimationState &state, QRegion &region, WindowPaintData &data) const
{
    const QPoint diff = state.geometry.topLeft() - w->geometry().topLeft();
    data.translate(diff.x(), diff.y());
}

void SlidingNotificationsEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt != m_animations.constEnd()) {
        paintAnimation(w, *animationIt, region, data);
        effects->paintWindow(w, mask, region, data);
        return;
    }

    auto stateIt = m_interAnimationState.constFind(w);
    if (stateIt != m_interAnimationState.constEnd()) {
        paintInterAnimation(w, *stateIt, region, data);
    }

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
            m_interAnimationState[w].geometry = animation.toGeometry;
            break;

        case AnimationKind::Move:
            m_interAnimationState[w].geometry = animation.toGeometry;
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
    const QRect windowRect = w->geometry();

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

    if (m_queuedAnimations.isEmpty()) {
        m_queuedAnimations.enqueue({w, animation});
        m_animations.insert(w, animation);
    } else if (m_queuedAnimations.head().animation.kind == animation.kind) {
        m_queuedAnimations.prepend({w, animation});
        m_animations.insert(w, animation);
    } else {
        m_queuedAnimations.enqueue({w, animation});
    }

    InterAnimationState &interState = m_interAnimationState[w];
    interState.geometry = animation.fromGeometry;

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
    const QRect windowRect = w->geometry();

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
    // TODO: Cleanup m_queuedAnimations.
    m_animations.remove(w);
    m_interAnimationState.remove(w);
}

void SlidingNotificationsEffect::slotWindowGeometryShapeChanged(EffectWindow *w, const QRect &old)
{
    if (!isNotificationWindow(w)) {
        return;
    }

    Animation animation;
    animation.kind = AnimationKind::Move;
    animation.fromGeometry = old;
    animation.toGeometry = w->geometry();
    animation.timeLine.setDirection(TimeLine::Forward);
    animation.timeLine.setDuration(m_duration);
    animation.timeLine.setEasingCurve(QEasingCurve::Linear);

    if (m_queuedAnimations.isEmpty()) {
        m_queuedAnimations.enqueue({w, animation});
        m_animations.insert(w, animation);
    } else if (m_queuedAnimations.head().animation.kind == animation.kind) {
        // TODO: What if it's already is active.
        m_animations.insert(w, animation);
    } else {
        m_queuedAnimations.enqueue({w, animation});
    }

    w->addRepaintFull();
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
    const QRect windowRect = w->geometry();

    const int centerX = screenRect.center().x();
    if (windowRect.left() < centerX && centerX < windowRect.right()) {
        return false;
    }

    return true;
}

SlidingNotificationsEffect::ScreenEdge SlidingNotificationsEffect::inferSlideScreenEdge(const EffectWindow *w) const
{
    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect windowRect = w->geometry();

    if (windowRect.right() < screenRect.center().x()) {
        return ScreenEdge::Left;
    }

    return ScreenEdge::Right;
}

void SlidingNotificationsEffect::startNextBatchOfAnimations()
{
    const AnimationKind currentAnimationKind = m_queuedAnimations.head().animation.kind;
    while (!m_queuedAnimations.isEmpty()
                && m_queuedAnimations.head().animation.kind == currentAnimationKind) {
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
