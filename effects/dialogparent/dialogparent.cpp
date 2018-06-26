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

// own
#include "dialogparent.h"

namespace KWin
{

DialogParentEffect::DialogParentEffect()
{
    reconfigure(ReconfigureAll);

    connect(effects, &EffectsHandler::windowAdded,
            this, &DialogParentEffect::windowAdded);
    connect(effects, &EffectsHandler::windowClosed,
            this, &DialogParentEffect::windowClosed);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &DialogParentEffect::windowDeleted);
    connect(effects, &EffectsHandler::windowModalityChanged,
            this, &DialogParentEffect::windowModalityChanged);
    connect(effects, &EffectsHandler::activeFullScreenEffectChanged,
            this, &DialogParentEffect::activeFullScreenEffectChanged);

    const auto windows = effects->stackingOrder();
    for (EffectWindow *w : windows) {
        if (!w->isModal()) {
            continue;
        }
        addModal(w);
    }
}

DialogParentEffect::~DialogParentEffect()
{
}

void DialogParentEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    m_duration = std::chrono::milliseconds(static_cast<int>(animationTime(250)));

    auto transitionIt = m_transitions.begin();
    while (transitionIt != m_transitions.end()) {
        (*transitionIt).timeLine.setDuration(m_duration);
        ++transitionIt;
    }
}

void DialogParentEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    const std::chrono::milliseconds delta(time);

    auto transitionIt = m_transitions.begin();
    while (transitionIt != m_transitions.end()) {
        (*transitionIt).timeLine.update(delta);
        ++transitionIt;
    }

    effects->prePaintScreen(data, time);
}

void DialogParentEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto transitionIt = m_transitions.constFind(w);
    if (transitionIt != m_transitions.constEnd()) {
        const qreal strength = (*transitionIt).dimStrength * (*transitionIt).timeLine.value();
        data.multiplyBrightness(1.0 - strength);
        data.multiplySaturation(1.0 - strength);
        effects->paintWindow(w, mask, region, data);
        return;
    }

    if (!effects->activeFullScreenEffect()) {
        auto dimIt = m_dimmedWindows.constFind(w);
        if (dimIt != m_dimmedWindows.constEnd()) {
            const qreal strength = (*dimIt);
            data.multiplyBrightness(1.0 - strength);
            data.multiplySaturation(1.0 - strength);
        }
    }

    effects->paintWindow(w, mask, region, data);
}

void DialogParentEffect::postPaintScreen()
{
    auto transitionIt = m_transitions.begin();
    while (transitionIt != m_transitions.end()) {
        EffectWindow *w = transitionIt.key();
        w->addRepaintFull();
        if ((*transitionIt).timeLine.done()) {
            transitionIt = m_transitions.erase(transitionIt);
        } else {
            ++transitionIt;
        }
    }

    effects->postPaintScreen();
}

bool DialogParentEffect::isActive() const
{
    return !m_dimmedWindows.isEmpty()
        || !m_transitions.isEmpty();
}

void DialogParentEffect::windowAdded(EffectWindow *w)
{
    if (w->isModal()) {
        addModal(w);
    }
}

void DialogParentEffect::windowClosed(EffectWindow *w)
{
    if (w->isModal()) {
        removeModal(w);
        return;
    }

    auto transitionIt = m_transitions.find(w);
    if (transitionIt != m_transitions.end()) {
        m_dimmedWindows[w] = (*transitionIt).dimStrength * (*transitionIt).timeLine.value();
        m_transitions.erase(transitionIt);
    }
}

void DialogParentEffect::windowDeleted(EffectWindow *w)
{
    m_dimmedWindows.remove(w);
    m_transitions.remove(w);
}

void DialogParentEffect::windowModalityChanged(EffectWindow *w)
{
    if (w->isModal()) {
        addModal(w);
    } else {
        removeModal(w);
    }
}

void DialogParentEffect::activeFullScreenEffectChanged()
{
    if (!isActive()) {
        return;
    }

    const auto direction = effects->activeFullScreenEffect()
        ? TimeLine::Backward
        : TimeLine::Forward;

    auto dimIt = m_dimmedWindows.constBegin();
    while (dimIt != m_dimmedWindows.constEnd()) {
        EffectWindow *w = dimIt.key();
        Transition &transition = m_transitions[w];

        if (transition.timeLine.running()) {
            transition.timeLine.setDirection(direction);
        } else {
            transition.dimStrength = (*dimIt);
            transition.timeLine.reset();
            transition.timeLine.setDuration(m_duration);
            transition.timeLine.setDirection(direction);
        }

        ++dimIt;
    }

    effects->addRepaintFull();
}

void DialogParentEffect::addModal(EffectWindow *m)
{
    const auto windows = m->mainWindows();
    for (EffectWindow *w : windows) {
        m_dimmedWindows[w] = m_dimStrength;
        w->addRepaintFull();

        if (effects->activeFullScreenEffect()) {
            continue;
        }

        Transition &transition = m_transitions[w];

        if (transition.timeLine.running()) {
            transition.timeLine.toggleDirection();
            continue;
        }

        transition.dimStrength = m_dimStrength;
        transition.timeLine.setDuration(m_duration);
        transition.timeLine.setDirection(TimeLine::Forward);
    }
}

void DialogParentEffect::removeModal(EffectWindow *m)
{
    const auto windows = m->mainWindows();
    for (EffectWindow *w : windows) {
        m_dimmedWindows.remove(w);
        w->addRepaintFull();

        if (effects->activeFullScreenEffect()) {
            continue;
        }

        Transition &transition = m_transitions[w];

        if (transition.timeLine.running()) {
            transition.timeLine.toggleDirection();
            continue;
        }

        transition.dimStrength = m_dimStrength;
        transition.timeLine.setDuration(m_duration);
        transition.timeLine.setDirection(TimeLine::Backward);
    }
}

} // namespace KWin
