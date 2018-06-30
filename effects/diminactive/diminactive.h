/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2007 Christian Nitschkowski <christian.nitschkowski@kdemail.net>
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

#ifndef KWIN_DIMINACTIVE_H
#define KWIN_DIMINACTIVE_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class DimInactiveEffect : public Effect
{
    Q_OBJECT

public:
    DimInactiveEffect();
    ~DimInactiveEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintScreen() override;

    int requestedEffectChainPosition() const override;
    bool isActive() const override;

private Q_SLOTS:
    void windowActivated(EffectWindow *w);
    void windowClosed(EffectWindow *w);
    void windowDeleted(EffectWindow *w);
    void activeFullScreenEffectChanged();

private:
    void dimWindow(WindowPaintData &data, qreal strength);
    bool canDimWindow(const EffectWindow *w) const;
    void scheduleInTransition(EffectWindow *w);
    void scheduleGroupInTransition(EffectWindow *w);
    void scheduleOutTransition(EffectWindow *w);
    void scheduleGroupOutTransition(EffectWindow *w);
    void scheduleRepaint(EffectWindow *w);

private:
    qreal m_dimStrength;
    bool m_dimPanels;
    bool m_dimDesktop;
    bool m_dimKeepAbove;
    bool m_dimByGroup;

    EffectWindow *m_activeWindow;
    const EffectWindowGroup *m_activeWindowGroup;
    QHash<EffectWindow*, TimeLine> m_transitions;
    QHash<EffectWindow*, qreal> m_forceDim;

    struct {
        bool active;
        TimeLine timeLine;
    } m_fullScreenTransition;
};

inline int DimInactiveEffect::requestedEffectChainPosition() const
{
    return 50;
}

inline bool DimInactiveEffect::isActive() const
{
    return true;
}

} // namespace KWin

#endif
