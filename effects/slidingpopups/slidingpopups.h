/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Marco Martin notmart@gmail.com

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

#ifndef KWIN_SLIDINGPOPUPS_H
#define KWIN_SLIDINGPOPUPS_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class SlidingPopupsEffect : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int slideInDuration READ slideInDuration)
    Q_PROPERTY(int slideOutDuration READ slideOutDuration)

public:
    SlidingPopupsEffect();
    ~SlidingPopupsEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintWindow(EffectWindow *w) override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    static bool supported();

    int slideInDuration() const;
    int slideOutDuration() const;

private Q_SLOTS:
    void windowAdded(EffectWindow *w);
    void windowDeleted(EffectWindow *w);
    void propertyNotify(EffectWindow *w, long atom);
    void waylandSlideOnShowChanged(EffectWindow *w);

    void slideIn(EffectWindow *w);
    void slideOut(EffectWindow *w);

private:
    void sanitizeAnimData(EffectWindow *w);

private:
    std::chrono::milliseconds m_slideInDuration;
    std::chrono::milliseconds m_slideOutDuration;
    int m_slideLength;

    enum class AnimationKind {
        In,
        Out
    };

    struct Animation {
        AnimationKind kind;
        TimeLine timeLine;
    };
    QHash<const EffectWindow*, Animation> m_animations;

    enum class Location {
        Top,
        Right,
        Bottom,
        Left
    };

    struct AnimationData {
        int offset;
        Location location;
        std::chrono::milliseconds slideInDuration;
        std::chrono::milliseconds slideOutDuration;
        int slideLength;
    };
    QHash<const EffectWindow*, AnimationData> m_animationData;

    long m_atom;
};

inline int SlidingPopupsEffect::requestedEffectChainPosition() const
{
    return 40;
}

inline int SlidingPopupsEffect::slideInDuration() const
{
    return m_slideInDuration.count();
}

inline int SlidingPopupsEffect::slideOutDuration() const
{
    return m_slideOutDuration.count();
}

} // namespace KWin

#endif
