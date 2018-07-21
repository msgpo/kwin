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

#ifndef KWIN_FADEDESKTOP_H
#define KWIN_FADEDESKTOP_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class FadeDesktopEffect : public Effect
{
    Q_OBJECT
    Q_PROPERTY(int duration READ duration)

public:
    FadeDesktopEffect();
    ~FadeDesktopEffect() override;

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
    void start(int oldDesktop, int newDesktop, EffectWindow *with);
    void stop();

    void windowAdded(EffectWindow *w);
    void windowDeleted(EffectWindow *w);

private:
    std::chrono::milliseconds m_duration;
    QHash<EffectWindow*, TimeLine> m_animations;
};

inline int FadeDesktopEffect::requestedEffectChainPosition() const
{
    return 50;
}

inline int FadeDesktopEffect::duration() const
{
    return m_duration.count();
}

} // namespace KWin

#endif
