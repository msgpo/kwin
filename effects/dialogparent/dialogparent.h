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

#ifndef KWIN_DIALOG_PARENT_H
#define KWIN_DIALOG_PARENT_H

// kwineffects
#include <kwineffects.h>

namespace KWin
{

class DialogParentEffect : public Effect
{
    Q_OBJECT

public:
    DialogParentEffect();
    ~DialogParentEffect() override;

    void reconfigure(ReconfigureFlags flags) override;

    void prePaintScreen(ScreenPrePaintData &data, int time) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;
    void postPaintScreen() override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override;

private Q_SLOTS:
    void windowAdded(EffectWindow *w);
    void windowClosed(EffectWindow *w);
    void windowDeleted(EffectWindow *w);
    void windowModalityChanged(EffectWindow *w);
    void activeFullScreenEffectChanged();

private:
    void addModal(EffectWindow *m);
    void removeModal(EffectWindow *m);

private:
    std::chrono::milliseconds m_duration;
    qreal m_dimStrength = 0.33;

    struct Transition {
        TimeLine timeLine;
        qreal dimStrength;
    };

    QHash<EffectWindow*, Transition> m_transitions;
    QHash<EffectWindow*, qreal> m_dimmedWindows;
};

inline int DialogParentEffect::requestedEffectChainPosition() const
{
    return 70;
}

} // namespace KWin

#endif
