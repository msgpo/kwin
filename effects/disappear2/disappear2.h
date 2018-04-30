/*
 * Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KWIN_EFFECTS_DISAPPEAR2_H
#define KWIN_EFFECTS_DISAPPEAR2_H

// kwineffects
#include <kwineffects.h>

// Qt
#include <QHash>
#include <QSet>
#include <QString>
#include <QTimeLine>

namespace KWin {

class Disappear2Effect : public Effect
{
public:
    Disappear2Effect();
    ~Disappear2Effect() override;

    void reconfigure(ReconfigureFlags flags) override;
    int requestedEffectChainPosition() const override;

    void prePaintScreen(ScreenPrePaintData& data, int time) override;
    void prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time) override;
    void paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data) override;
    void postPaintScreen() override;

    bool isActive() const override;

public:
    static bool supported();

private:
    bool shouldAnimate(const EffectWindow* w) const;
    void start(EffectWindow* w);
    void stop(EffectWindow* w);
    void markWindow(EffectWindow* w);

private:
    QSet<QString> m_blacklist;
    QHash<EffectWindow*, QTimeLine*> m_animations;
    int m_duration;
    qreal m_opacity;
    qreal m_pitch;
    qreal m_distance;
};

inline
int Disappear2Effect::requestedEffectChainPosition() const
{
    return 50;
}

} // namespace KWin

#endif // KWIN_EFFECTS_DISAPPEAR2_H
