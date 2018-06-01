/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Philip Falkner <philip.falkner@gmail.com>
Copyright (C) 2009 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2010 Alexandre Pereira <pereira.alex@gmail.com>
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

#ifndef KWIN_GLIDE_H
#define KWIN_GLIDE_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class GlideEffect : public Effect
{
    Q_OBJECT

public:
    GlideEffect();
    ~GlideEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintScreen() override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

    static bool supported();

private Q_SLOTS:
    void windowAdded(EffectWindow *w);
    void windowClosed(EffectWindow *w);
    void windowDeleted(EffectWindow *w);
    void windowDataChanged(EffectWindow *w, int role);

private:
    bool isGlideWindow(EffectWindow *w) const;

    std::chrono::milliseconds m_duration;
    QHash<EffectWindow*, TimeLine> m_animations;

    enum RotationEdge {
        Top = 0,
        Right = 1,
        Bottom = 2,
        Left = 3
    };

    struct GlideParams {
        int edge;
        struct {
            qreal from;
            qreal to;
        } angle, distance, opacity;
    };

    GlideParams m_inParams;
    GlideParams m_outParams;
};

inline int GlideEffect::requestedEffectChainPosition() const
{
    return 50;
}

} // namespace KWin

#endif
