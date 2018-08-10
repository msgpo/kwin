/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2008 Martin Gräßlin <mgraesslin@kde.org>
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
#include "magiclamp.h"

// KConfigSkeleton
#include "magiclampconfig.h"

// std
#include <cmath>

namespace KWin
{

static inline std::chrono::milliseconds durationFraction(std::chrono::milliseconds duration, qreal fraction)
{
    return std::chrono::milliseconds(qMax(qRound(duration.count() * fraction), 1));
}

MagicLampEffect::MagicLampEffect()
{
    initConfig<MagicLampConfig>();
    reconfigure(ReconfigureAll);

    m_shapeCurve.setType(QEasingCurve::InOutSine);

    connect(effects, &EffectsHandler::windowMinimized,
            this, &MagicLampEffect::slotWindowMinimized);
    connect(effects, &EffectsHandler::windowUnminimized,
            this, &MagicLampEffect::slotWindowUnminimized);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &MagicLampEffect::slotWindowDeleted);
    connect(effects, &EffectsHandler::activeFullScreenEffectChanged,
            this, &MagicLampEffect::slotActiveFullScreenEffectChanged);
}

void MagicLampEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    MagicLampConfig::self()->read();

    // TODO: Rename AnimationDuration config key to Duration.
    const int rawDuration = MagicLampConfig::animationDuration();
    const int baseDuration = static_cast<int>(animationTime(rawDuration ? rawDuration : 250));

    m_squashDuration = std::chrono::milliseconds(baseDuration);
    m_stretchDuration = std::chrono::milliseconds(qMax(qRound(baseDuration * 0.4), 1));
    m_bumpDuration = std::chrono::milliseconds(qMax(qRound(baseDuration * 0.9), 1));
}

void MagicLampEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    const std::chrono::milliseconds delta(time);

    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        (*animationIt).timeLine.update(delta);
        ++animationIt;
    }

    // We need to mark the screen windows as transformed. Otherwise the
    // whole screen won't be repainted, resulting in artefacts.
    data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;

    effects->prePaintScreen(data, time);
}

void MagicLampEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt != m_animations.constEnd()) {
        data.setTransformed();
        w->enablePainting(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);

        // Windows are transformed in Stretch1, Stretch2, and Squash stages.
        if ((*animationIt).stage != AnimationStage::Bump) {
            data.quads = data.quads.makeGrid(m_gridResolution);
        }
    }

    effects->prePaintWindow(w, data, time);
}

void MagicLampEffect::paintBumpStage(const Animation &animation, WindowPaintData &data) const
{
    const qreal t = animation.timeLine.value();

    switch (animation.direction) {
    case Direction::Top:
        data.translate(0.0, interpolate(0.0, animation.bumpDistance, t));
        break;

    case Direction::Right:
        data.translate(-interpolate(0.0, animation.bumpDistance, t), 0.0);
        break;

    case Direction::Bottom:
        data.translate(0.0, -interpolate(0.0, animation.bumpDistance, t));
        break;

    case Direction::Left:
        data.translate(interpolate(0.0, animation.bumpDistance, t), 0.0);
        break;

    default:
        Q_UNREACHABLE();
    }
}

WindowQuadList MagicLampEffect::transformTop(qreal stretchProgress, qreal squashProgress,
                                             qreal bumpProgress, qreal bumpDistance,
                                             const QRect &windowRect, const QRect &iconRect,
                                             const WindowQuadList &quads) const
{
    WindowQuadList transformedQuads;
    transformedQuads.reserve(quads.size());

    const qreal distance = windowRect.bottom() - iconRect.bottom() + bumpDistance;

    for (const WindowQuad &q : quads) {
        const qreal topOffset = q[0].y() - interpolate(0.0, distance, squashProgress);
        const qreal bottomOffset = q[2].y() - interpolate(0.0, distance, squashProgress);

        const qreal topScale = stretchProgress * m_shapeCurve.valueForProgress((windowRect.height() - topOffset) / distance);
        const qreal bottomScale = stretchProgress * m_shapeCurve.valueForProgress((windowRect.height() - bottomOffset) / distance);

        WindowQuad newQuad(q);

        const qreal targetTopLeftX = iconRect.x() + iconRect.width() * q[0].x() / windowRect.width();
        const qreal targetTopRightX = iconRect.x() + iconRect.width() * q[1].x() / windowRect.width();
        const qreal targetBottomRightX = iconRect.x() + iconRect.width() * q[2].x() / windowRect.width();
        const qreal targetBottomLeftX = iconRect.x() + iconRect.width() * q[3].x() / windowRect.width();

        newQuad[0].setX(q[0].x() + topScale * (targetTopLeftX - (windowRect.x() + q[0].x())));
        newQuad[1].setX(q[1].x() + topScale * (targetTopRightX - (windowRect.x() + q[1].x())));
        newQuad[2].setX(q[2].x() + bottomScale * (targetBottomRightX - (windowRect.x() + q[2].x())));
        newQuad[3].setX(q[3].x() + bottomScale * (targetBottomLeftX - (windowRect.x() + q[3].x())));

        const qreal targetTopOffset = topOffset + bumpDistance * bumpProgress;
        const qreal targetBottomOffset = bottomOffset + bumpDistance * bumpProgress;

        newQuad[0].setY(targetTopOffset);
        newQuad[1].setY(targetTopOffset);
        newQuad[2].setY(targetBottomOffset);
        newQuad[3].setY(targetBottomOffset);

        transformedQuads.append(newQuad);
    }

    return transformedQuads;
}

WindowQuadList MagicLampEffect::transformBottom(qreal stretchProgress, qreal squashProgress,
                                                qreal bumpProgress, qreal bumpDistance,
                                                const QRect &windowRect, const QRect &iconRect,
                                                const WindowQuadList &quads) const
{
    WindowQuadList transformedQuads;
    transformedQuads.reserve(quads.size());

    const qreal distance = iconRect.top() - windowRect.top() + bumpDistance;

    for (const WindowQuad &q : quads) {
        const qreal topOffset = q[0].y() + interpolate(0.0, distance, squashProgress);
        const qreal bottomOffset = q[2].y() + interpolate(0.0, distance, squashProgress);

        const qreal topScale = stretchProgress * m_shapeCurve.valueForProgress(topOffset / distance);
        const qreal bottomScale = stretchProgress * m_shapeCurve.valueForProgress(bottomOffset / distance);

        WindowQuad newQuad(q);

        const qreal targetTopLeftX = iconRect.x() + iconRect.width() * q[0].x() / windowRect.width();
        const qreal targetTopRightX = iconRect.x() + iconRect.width() * q[1].x() / windowRect.width();
        const qreal targetBottomRightX = iconRect.x() + iconRect.width() * q[2].x() / windowRect.width();
        const qreal targetBottomLeftX = iconRect.x() + iconRect.width() * q[3].x() / windowRect.width();

        newQuad[0].setX(q[0].x() + topScale * (targetTopLeftX - (windowRect.x() + q[0].x())));
        newQuad[1].setX(q[1].x() + topScale * (targetTopRightX - (windowRect.x() + q[1].x())));
        newQuad[2].setX(q[2].x() + bottomScale * (targetBottomRightX - (windowRect.x() + q[2].x())));
        newQuad[3].setX(q[3].x() + bottomScale * (targetBottomLeftX - (windowRect.x() + q[3].x())));

        const qreal targetTopOffset = topOffset - bumpDistance * bumpProgress;
        const qreal targetBottomOffset = bottomOffset - bumpDistance * bumpProgress;

        newQuad[0].setY(targetTopOffset);
        newQuad[1].setY(targetTopOffset);
        newQuad[2].setY(targetBottomOffset);
        newQuad[3].setY(targetBottomOffset);

        transformedQuads.append(newQuad);
    }

    return transformedQuads;
}

WindowQuadList MagicLampEffect::transformLeft(qreal stretchProgress, qreal squashProgress,
                                              qreal bumpProgress, qreal bumpDistance,
                                              const QRect &windowRect, const QRect &iconRect,
                                              const WindowQuadList &quads) const
{
    WindowQuadList transformedQuads;
    transformedQuads.reserve(quads.size());

    const qreal distance = windowRect.right() - iconRect.right() + bumpDistance;

    for (const WindowQuad &q : quads) {
        const qreal leftOffset = q[0].x() - interpolate(0.0, distance, squashProgress);
        const qreal rightOffset = q[2].x() - interpolate(0.0, distance, squashProgress);

        const qreal leftScale = stretchProgress * m_shapeCurve.valueForProgress((windowRect.width() - leftOffset) / distance);
        const qreal rightScale = stretchProgress * m_shapeCurve.valueForProgress((windowRect.width() - rightOffset) / distance);

        WindowQuad newQuad(q);

        const qreal targetTopLeftY = iconRect.y() + iconRect.height() * q[0].y() / windowRect.height();
        const qreal targetTopRightY = iconRect.y() + iconRect.height() * q[1].y() / windowRect.height();
        const qreal targetBottomRightY = iconRect.y() + iconRect.height() * q[2].y() / windowRect.height();
        const qreal targetBottomLeftY = iconRect.y() + iconRect.height() * q[3].y() / windowRect.height();

        newQuad[0].setY(q[0].y() + leftScale * (targetTopLeftY - (windowRect.y() + q[0].y())));
        newQuad[3].setY(q[3].y() + leftScale * (targetBottomLeftY - (windowRect.y() + q[3].y())));
        newQuad[1].setY(q[1].y() + rightScale * (targetTopRightY - (windowRect.y() + q[1].y())));
        newQuad[2].setY(q[2].y() + rightScale * (targetBottomRightY - (windowRect.y() + q[2].y())));

        const qreal targetLeftOffset = leftOffset + bumpDistance * bumpProgress;
        const qreal targetRightOffset = rightOffset + bumpDistance * bumpProgress;

        newQuad[0].setX(targetLeftOffset);
        newQuad[3].setX(targetLeftOffset);
        newQuad[1].setX(targetRightOffset);
        newQuad[2].setX(targetRightOffset);

        transformedQuads.append(newQuad);
    }

    return transformedQuads;
}

WindowQuadList MagicLampEffect::transformRight(qreal stretchProgress, qreal squashProgress,
                                               qreal bumpProgress, qreal bumpDistance,
                                               const QRect &windowRect, const QRect &iconRect,
                                               const WindowQuadList &quads) const
{
    WindowQuadList transformedQuads;
    transformedQuads.reserve(quads.size());

    const qreal distance = iconRect.left() - windowRect.left() + bumpDistance;

    for (const WindowQuad &q : quads) {
        const qreal leftOffset = q[0].x() + interpolate(0.0, distance, squashProgress);
        const qreal rightOffset = q[2].x() + interpolate(0.0, distance, squashProgress);

        const qreal leftScale = stretchProgress * m_shapeCurve.valueForProgress(leftOffset / distance);
        const qreal rightScale = stretchProgress * m_shapeCurve.valueForProgress(rightOffset / distance);

        WindowQuad newQuad(q);

        const qreal targetTopLeftY = iconRect.y() + iconRect.height() * q[0].y() / windowRect.height();
        const qreal targetTopRightY = iconRect.y() + iconRect.height() * q[1].y() / windowRect.height();
        const qreal targetBottomRightY = iconRect.y() + iconRect.height() * q[2].y() / windowRect.height();
        const qreal targetBottomLeftY = iconRect.y() + iconRect.height() * q[3].y() / windowRect.height();

        newQuad[0].setY(q[0].y() + leftScale * (targetTopLeftY - (windowRect.y() + q[0].y())));
        newQuad[3].setY(q[3].y() + leftScale * (targetBottomLeftY - (windowRect.y() + q[3].y())));
        newQuad[1].setY(q[1].y() + rightScale * (targetTopRightY - (windowRect.y() + q[1].y())));
        newQuad[2].setY(q[2].y() + rightScale * (targetBottomRightY - (windowRect.y() + q[2].y())));

        const qreal targetLeftOffset = leftOffset - bumpDistance * bumpProgress;
        const qreal targetRightOffset = rightOffset - bumpDistance * bumpProgress;

        newQuad[0].setX(targetLeftOffset);
        newQuad[3].setX(targetLeftOffset);
        newQuad[1].setX(targetRightOffset);
        newQuad[2].setX(targetRightOffset);

        transformedQuads.append(newQuad);
    }

    return transformedQuads;
}

WindowQuadList MagicLampEffect::transformGeneric(Direction direction, qreal stretchProgress,
                                                 qreal squashProgress, qreal bumpProgress,
                                                 qreal bumpDistance, const QRect &windowRect,
                                                 const QRect &iconRect, const WindowQuadList &quads) const
{
    switch (direction) {
    case Direction::Top:
        return transformTop(stretchProgress, squashProgress, bumpProgress,
            bumpDistance, windowRect, iconRect, quads);

    case Direction::Right:
        return transformRight(stretchProgress, squashProgress, bumpProgress,
            bumpDistance, windowRect, iconRect, quads);

    case Direction::Bottom:
        return transformBottom(stretchProgress, squashProgress, bumpProgress,
            bumpDistance, windowRect, iconRect, quads);

    case Direction::Left:
        return transformLeft(stretchProgress, squashProgress, bumpProgress,
            bumpDistance, windowRect, iconRect, quads);

    default:
        Q_UNREACHABLE();
    }
}

void MagicLampEffect::paintSquashStage(const EffectWindow *w, const Animation &animation, WindowPaintData &data) const
{
    const QRect windowRect = w->geometry();
    const QRect iconRect = w->iconGeometry();

    const qreal squashProgress = animation.timeLine.value();
    const qreal stretchProgress = qMin(animation.stretchFactor + squashProgress, 1.0);
    const qreal bumpProgress = 1.0;

    data.quads = transformGeneric(animation.direction, stretchProgress, squashProgress,
        bumpProgress, animation.bumpDistance, windowRect, iconRect, data.quads);
}

void MagicLampEffect::paintStretch1Stage(const EffectWindow *w, const Animation &animation, WindowPaintData &data) const
{
    const QRect windowRect = w->geometry();
    const QRect iconRect = w->iconGeometry();

    const qreal stretchProgress = animation.stretchFactor * animation.timeLine.value();
    const qreal squashProgress = 0.0;
    const qreal bumpProgress = 1.0;

    data.quads = transformGeneric(animation.direction, stretchProgress, squashProgress,
        bumpProgress, animation.bumpDistance, windowRect, iconRect, data.quads);
}

void MagicLampEffect::paintStretch2Stage(const EffectWindow *w, const Animation &animation, WindowPaintData &data) const
{
    const QRect windowRect = w->geometry();
    const QRect iconRect = w->iconGeometry();

    const qreal stretchProgress = animation.stretchFactor * animation.timeLine.value();
    const qreal squashProgress = 0.0;
    const qreal bumpProgress = stretchProgress;

    data.quads = transformGeneric(animation.direction, stretchProgress, squashProgress,
        bumpProgress, animation.bumpDistance, windowRect, iconRect, data.quads);
}

void MagicLampEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt == m_animations.constEnd()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    switch ((*animationIt).stage) {
    case AnimationStage::Bump:
        paintBumpStage(*animationIt, data);
        break;

    case AnimationStage::Stretch1:
        paintStretch1Stage(w, *animationIt, data);
        break;

    case AnimationStage::Stretch2:
        paintStretch2Stage(w, *animationIt, data);
        break;

    case AnimationStage::Squash:
        paintSquashStage(w, *animationIt, data);
        break;

    default:
        Q_UNREACHABLE();
    }

    if ((*animationIt).clip) {
        const QRect iconRect = w->iconGeometry();
        QRect clipRect = w->expandedGeometry();
        switch ((*animationIt).direction) {
        case Direction::Top:
            clipRect.translate(0, (*animationIt).bumpDistance);
            clipRect.setTop(iconRect.top());
            clipRect.setLeft(qMin(iconRect.left(), clipRect.left()));
            clipRect.setRight(qMax(iconRect.right(), clipRect.right()));
            break;

        case Direction::Right:
            clipRect.translate(-(*animationIt).bumpDistance, 0);
            clipRect.setRight(iconRect.right());
            clipRect.setTop(qMin(iconRect.top(), clipRect.top()));
            clipRect.setBottom(qMax(iconRect.bottom(), clipRect.bottom()));
            break;

        case Direction::Bottom:
            clipRect.translate(0, -(*animationIt).bumpDistance);
            clipRect.setBottom(iconRect.bottom());
            clipRect.setLeft(qMin(iconRect.left(), clipRect.left()));
            clipRect.setRight(qMax(iconRect.right(), clipRect.right()));
            break;

        case Direction::Left:
            clipRect.translate((*animationIt).bumpDistance, 0);
            clipRect.setLeft(iconRect.left());
            clipRect.setTop(qMin(iconRect.top(), clipRect.top()));
            clipRect.setBottom(qMax(iconRect.bottom(), clipRect.bottom()));
            break;

        default:
            Q_UNREACHABLE();
        }

        region = QRegion(clipRect);
    }

    effects->paintWindow(w, mask, region, data);
}

bool MagicLampEffect::updateInAnimationStage(Animation &animation)
{
    switch (animation.stage) {
    case AnimationStage::Bump:
        animation.stage = AnimationStage::Stretch1;
        animation.timeLine.reset();
        animation.timeLine.setDirection(TimeLine::Forward);
        animation.timeLine.setDuration(
            durationFraction(m_stretchDuration, animation.stretchFactor));
        animation.timeLine.setEasingCurve(QEasingCurve::Linear);
        animation.clip = true;
        return false;

    case AnimationStage::Stretch1:
    case AnimationStage::Stretch2:
        animation.stage = AnimationStage::Squash;
        animation.timeLine.reset();
        animation.timeLine.setDirection(TimeLine::Forward);
        animation.timeLine.setDuration(m_squashDuration);
        animation.timeLine.setEasingCurve(QEasingCurve::Linear);
        animation.clip = true;
        return false;

    case AnimationStage::Squash:
        return true;

    default:
        Q_UNREACHABLE();
    }
}

bool MagicLampEffect::updateOutAnimationStage(Animation &animation)
{
    switch (animation.stage) {
    case AnimationStage::Bump:
        return true;

    case AnimationStage::Stretch1:
        if (animation.bumpDistance == 0) {
            return true;
        }
        animation.stage = AnimationStage::Bump;
        animation.timeLine.reset();
        animation.timeLine.setDirection(TimeLine::Backward);
        animation.timeLine.setDuration(m_bumpDuration);
        animation.timeLine.setEasingCurve(QEasingCurve::Linear);
        animation.clip = false;
        return false;

    case AnimationStage::Stretch2:
        return true;

    case AnimationStage::Squash:
        animation.stage = AnimationStage::Stretch2;
        animation.timeLine.reset();
        animation.timeLine.setDirection(TimeLine::Backward);
        animation.timeLine.setDuration(
            durationFraction(m_stretchDuration, animation.stretchFactor));
        animation.timeLine.setEasingCurve(QEasingCurve::Linear);
        animation.clip = false;
        return false;

    default:
        Q_UNREACHABLE();
    }
}

void MagicLampEffect::postPaintScreen()
{
    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        if (!(*animationIt).timeLine.done()) {
            ++animationIt;
            continue;
        }

        bool done;
        switch ((*animationIt).kind) {
        case AnimationKind::In:
            done = updateInAnimationStage(*animationIt);
            break;

        case AnimationKind::Out:
            done = updateOutAnimationStage(*animationIt);
            break;

        default:
            Q_UNREACHABLE();
        }

        if (done) {
            animationIt = m_animations.erase(animationIt);
        } else {
            ++animationIt;
        }
    }

    effects->addRepaintFull();

    effects->postPaintScreen();
}

bool MagicLampEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool MagicLampEffect::supported()
{
    return effects->isOpenGLCompositing()
        && effects->animationsSupported();
}

void MagicLampEffect::slotWindowMinimized(EffectWindow *w)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    const QRect iconRect = w->iconGeometry();
    if (!iconRect.isValid()) {
        return;
    }

    Animation &animation = m_animations[w];
    animation.kind = AnimationKind::In;

    if (animation.timeLine.running()) {
        animation.timeLine.toggleDirection();
        // Don't need to schedule repaint because it's already scheduled.
        return;
    }

    animation.direction = findDirectionToIcon(w);
    animation.bumpDistance = calcBumpDistance(w, animation.direction);
    animation.stretchFactor = calcStretchFactor(w, animation.direction, animation.bumpDistance);

    if (animation.bumpDistance != 0) {
        animation.stage = AnimationStage::Bump;
        animation.timeLine.reset();
        animation.timeLine.setDirection(TimeLine::Forward);
        animation.timeLine.setDuration(m_bumpDuration);
        animation.timeLine.setEasingCurve(QEasingCurve::Linear);
        animation.clip = false;
    } else {
        animation.stage = AnimationStage::Stretch1;
        animation.timeLine.reset();
        animation.timeLine.setDirection(TimeLine::Forward);
        animation.timeLine.setDuration(
            durationFraction(m_stretchDuration, animation.stretchFactor));
        animation.timeLine.setEasingCurve(QEasingCurve::Linear);
        animation.clip = true;
    }

    effects->addRepaintFull();
}

void MagicLampEffect::slotWindowUnminimized(EffectWindow *w)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    const QRect iconRect = w->iconGeometry();
    if (!iconRect.isValid()) {
        return;
    }

    Animation &animation = m_animations[w];
    animation.kind = AnimationKind::Out;

    if (animation.timeLine.running()) {
        animation.timeLine.toggleDirection();
        // Don't need to schedule repaint because it's already scheduled.
        return;
    }

    animation.direction = findDirectionToIcon(w);
    animation.bumpDistance = calcBumpDistance(w, animation.direction);
    animation.stretchFactor = calcStretchFactor(w, animation.direction, animation.bumpDistance);

    animation.stage = AnimationStage::Squash;
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Backward);
    animation.timeLine.setDuration(m_squashDuration);
    animation.timeLine.setEasingCurve(QEasingCurve::Linear);
    animation.clip = true;

    effects->addRepaintFull();
}

void MagicLampEffect::slotWindowDeleted(EffectWindow *w)
{
    m_animations.remove(w);
}

void MagicLampEffect::slotActiveFullScreenEffectChanged()
{
    if (effects->activeFullScreenEffect() == nullptr) {
        return;
    }

    m_animations.clear();
}

MagicLampEffect::Direction MagicLampEffect::findDirectionToIcon(const EffectWindow *w) const
{
    const QRect iconRect = w->iconGeometry();

    const EffectWindowList windows = effects->stackingOrder();
    auto panelIt = std::find_if(windows.constBegin(), windows.constEnd(),
        [&iconRect](const EffectWindow *w) {
            if (!w->isDock()) {
                return false;
            }
            return w->geometry().intersects(iconRect);
        });
    const EffectWindow *panel = (panelIt != windows.constEnd())
        ? (*panelIt)
        : nullptr;

    Direction direction;
    if (panel != nullptr) {
        const QRect panelScreen = effects->clientArea(ScreenArea, (*panelIt));
        if (panel->width() >= panel->height()) {
            direction = (panel->y() == panelScreen.y())
                ? Direction::Top
                : Direction::Bottom;
        } else {
            direction = (panel->x() == panelScreen.x())
                ? Direction::Left
                : Direction::Right;
        }
    } else {
        const QRect iconScreen = effects->clientArea(ScreenArea, iconRect.center(), effects->currentDesktop());
        const QRect rect = iconScreen.intersected(iconRect);

        // TODO: Explain why this is in some sense wrong.
        if (rect.left() == iconScreen.left()) {
            direction = Direction::Left;
        } else if (rect.top() == iconScreen.top()) {
            direction = Direction::Top;
        } else if (rect.right() == iconScreen.right()) {
            direction = Direction::Right;
        } else {
            direction = Direction::Bottom;
        }
    }

    if (panel != nullptr && panel->screen() == w->screen()) {
        return direction;
    }

    const QRect windowRect = w->geometry();

    if (direction == Direction::Top && windowRect.top() < iconRect.top()) {
        direction = Direction::Bottom;
    } else if (direction == Direction::Right && iconRect.right() < windowRect.right()) {
        direction = Direction::Left;
    } else if (direction == Direction::Bottom && iconRect.bottom() < windowRect.bottom()) {
        direction = Direction::Top;
    } else if (direction == Direction::Left && windowRect.left() < iconRect.left()) {
        direction = Direction::Right;
    }

    return direction;
}

int MagicLampEffect::calcBumpDistance(const EffectWindow *w, Direction direction) const
{
    const QRect windowRect = w->geometry();
    const QRect iconRect = w->iconGeometry();

    int bumpDistance = 0;
    switch (direction) {
    case Direction::Top:
        bumpDistance = qMax(0, iconRect.y() + iconRect.height() - windowRect.y());
        break;

    case Direction::Right:
        bumpDistance = qMax(0, windowRect.x() + windowRect.width() - iconRect.x());
        break;

    case Direction::Bottom:
        bumpDistance = qMax(0, windowRect.y() + windowRect.height() - iconRect.y());
        break;

    case Direction::Left:
        bumpDistance = qMax(0, iconRect.x() + iconRect.width() - windowRect.x());
        break;

    default:
        Q_UNREACHABLE();
    }

    bumpDistance += qMin(bumpDistance, m_maxBumpDistance);

    return bumpDistance;
}

qreal MagicLampEffect::calcStretchFactor(const EffectWindow *w, Direction direction, int bumpDistance) const
{
    const QRect windowRect = w->geometry();
    const QRect iconRect = w->iconGeometry();

    int movingExtent = 0;
    int distanceToIcon = 0;
    switch (direction) {
    case Direction::Top:
        movingExtent = windowRect.height();
        distanceToIcon = windowRect.bottom() - iconRect.bottom() + bumpDistance;
        break;

    case Direction::Right:
        movingExtent = windowRect.width();
        distanceToIcon = iconRect.left() - windowRect.left() + bumpDistance;
        break;

    case Direction::Bottom:
        movingExtent = windowRect.height();
        distanceToIcon = iconRect.top() - windowRect.top() + bumpDistance;
        break;

    case Direction::Left:
        movingExtent = windowRect.width();
        distanceToIcon = windowRect.right() - iconRect.right() + bumpDistance;
        break;

    default:
        Q_UNREACHABLE();
    }

    return static_cast<qreal>(movingExtent) / distanceToIcon;
}

} // namespace KWin
