/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2008, 2009 Martin Gräßlin <mgraesslin@kde.org>
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

#ifndef KWIN_DIMSCREEN_H
#define KWIN_DIMSCREEN_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class DimScreenEffect : public Effect
{
    Q_OBJECT

public:
    DimScreenEffect();
    ~DimScreenEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintScreen() override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

private Q_SLOTS:
    void windowActivated(EffectWindow *w);
    void activeFullScreenEffectChanged();

private:
    bool canDim(const EffectWindow *w) const;

    qreal m_dimStrength = 0.33;

    EffectWindow *m_authDialog = nullptr;
    qreal m_currentDimStrength;

    struct Transition {
        bool active = false;
        TimeLine timeLine;
    };

    Transition m_dimTransition;
};

inline int DimScreenEffect::requestedEffectChainPosition() const
{
    return 50;
}

} // namespace KWin

#endif
