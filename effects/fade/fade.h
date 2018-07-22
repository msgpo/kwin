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

#ifndef KWIN_FADE_H
#define KWIN_FADE_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class FadeEffect : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int fadeInDuration READ fadeInDuration)
    Q_PROPERTY(int fadeOutDuration READ fadeOutDuration)

public:
    FadeEffect();
    ~FadeEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintScreen() override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    static bool supported();

    int fadeInDuration() const;
    int fadeOutDuration() const;

private Q_SLOTS:
    void fadeIn(EffectWindow *w);
    void fadeOut(EffectWindow *w);

    void windowDeleted(EffectWindow *w);
    void windowDataChanged(EffectWindow *w, int role);
    void windowMinimized(EffectWindow *w);

private:
    std::chrono::milliseconds m_fadeInDuration;
    std::chrono::milliseconds m_fadeOutDuration;

    enum class AnimationKind {
        In,
        Out
    };

    struct Animation {
        AnimationKind kind;
        TimeLine timeLine;
    };

    QHash<EffectWindow*, Animation> m_animations;
    QSet<const EffectWindow*> m_tempFadeInBlacklist;
};

inline int FadeEffect::requestedEffectChainPosition() const
{
    return 60;
}

inline int FadeEffect::fadeInDuration() const
{
    return m_fadeInDuration.count();
}

inline int FadeEffect::fadeOutDuration() const
{
    return m_fadeOutDuration.count();
}

} // namespace KWin

#endif
