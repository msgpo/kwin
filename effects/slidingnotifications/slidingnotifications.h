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

#pragma once

// kwineffects
#include <kwineffects.h>

// Qt
#include <QQueue>

namespace KWin
{

class SlidingNotificationsEffect : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration)

public:
    SlidingNotificationsEffect();

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintScreen() override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    static bool supported();

    int duration() const;

private Q_SLOTS:
    void slotWindowAdded(EffectWindow *w);
    void slotWindowClosed(EffectWindow *w);
    void slotWindowDeleted(EffectWindow *w);
    void slotWindowGeometryShapeChanged(EffectWindow *w, const QRect &old);

private:
    bool isNotificationWindow(const EffectWindow *w) const;

private:
    std::chrono::milliseconds m_duration;

    enum class AnimationKind {
        SlideIn,
        SlideOut,
        Move
    };

    enum class ScreenEdge {
        Top,
        Right,
        Bottom,
        Left
    };

    struct Animation {
        AnimationKind kind;
        QRect fromGeometry;
        QRect toGeometry;
        ScreenEdge screenEdge;
        TimeLine timeLine;
    };

    struct QueuedAnimation {
        EffectWindow *target;
        Animation animation;
    };

    QQueue<QueuedAnimation> m_queuedAnimations;
    QHash<EffectWindow *, Animation> m_animations;
};

inline int SlidingNotificationsEffect::requestedEffectChainPosition() const
{
    return 60;
}

inline int SlidingNotificationsEffect::duration() const
{
    return m_duration.count();
}

} // namespace KWin
