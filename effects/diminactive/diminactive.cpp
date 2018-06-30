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

// own
#include "diminactive.h"

// KConfigSkeleton
#include "diminactiveconfig.h"

namespace KWin
{

/**
 * Checks if two windows belong to the same window group
 *
 * One possible example of a window group is an app window and app
 * preferences window(e.g. Dolphin window and Dolphin Preferences window).
 *
 * @param w1 The first window
 * @param w2 The second window
 * @returns @c true if both windows belong to the same window group, @c false otherwise
 **/
static inline bool belongToSameGroup(const EffectWindow *w1, const EffectWindow *w2)
{
    return w1 && w2 && w1->group() && w1->group() == w2->group();
}

DimInactiveEffect::DimInactiveEffect()
{
    m_activeWindow = nullptr;

    m_fullScreenTransition.active = false;
    m_fullScreenTransition.timeLine.setDirection(TimeLine::Forward);
    m_fullScreenTransition.timeLine.setDuration(
        std::chrono::milliseconds(static_cast<int>(animationTime(250))));
    m_fullScreenTransition.timeLine.setEasingCurve(QEasingCurve::InOutQuad);

    initConfig<DimInactiveConfig>();
    reconfigure(ReconfigureAll);

    connect(effects, &EffectsHandler::windowActivated,
            this, &DimInactiveEffect::windowActivated);
    connect(effects, &EffectsHandler::windowClosed,
            this, &DimInactiveEffect::windowClosed);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &DimInactiveEffect::windowDeleted);
    connect(effects, &EffectsHandler::activeFullScreenEffectChanged,
            this, &DimInactiveEffect::activeFullScreenEffectChanged);
}

DimInactiveEffect::~DimInactiveEffect()
{
}

void DimInactiveEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    DimInactiveConfig::self()->read();
    m_dimStrength = DimInactiveConfig::strength() / 100.0;
    m_dimPanels = DimInactiveConfig::dimPanels();
    m_dimDesktop = DimInactiveConfig::dimDesktop();
    m_dimKeepAbove = DimInactiveConfig::dimKeepAbove();
    m_dimByGroup = DimInactiveConfig::dimByGroup();

    EffectWindow *activeWindow = effects->activeWindow();
    m_activeWindow = (activeWindow && canDimWindow(activeWindow))
        ? activeWindow
        : nullptr;

    m_activeWindowGroup = (m_dimByGroup && m_activeWindow)
        ? m_activeWindow->group()
        : nullptr;

    m_fullScreenTransition.timeLine.setDuration(
        std::chrono::milliseconds(static_cast<int>(animationTime(250))));

    effects->addRepaintFull();
}

void DimInactiveEffect::prePaintScreen(ScreenPrePaintData &data, int time)
{
    const std::chrono::milliseconds delta(time);

    if (m_fullScreenTransition.active) {
        m_fullScreenTransition.timeLine.update(delta);
    }

    auto transitionIt = m_transitions.begin();
    while (transitionIt != m_transitions.end()) {
        (*transitionIt).update(delta);
        ++transitionIt;
    }

    effects->prePaintScreen(data, time);
}

void DimInactiveEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto forceIt = m_forceDim.constFind(w);
    if (forceIt != m_forceDim.constEnd()) {
        const qreal forcedStrength = *forceIt;
        dimWindow(data, forcedStrength);
        effects->paintWindow(w, mask, region, data);
        return;
    }

    auto transitionIt = m_transitions.constFind(w);
    if (transitionIt != m_transitions.constEnd()) {
        const qreal transitionProgress = (*transitionIt).value();
        dimWindow(data, m_dimStrength * transitionProgress);
        effects->paintWindow(w, mask, region, data);
        return;
    }

    if (canDimWindow(w)) {
        dimWindow(data, m_dimStrength);
    }

    effects->paintWindow(w, mask, region, data);
}

void DimInactiveEffect::postPaintScreen()
{
    if (m_fullScreenTransition.active) {
        if (m_fullScreenTransition.timeLine.done()) {
            m_fullScreenTransition.active = false;
        }
        effects->addRepaintFull();
    }

    auto transitionIt = m_transitions.begin();
    while (transitionIt != m_transitions.end()) {
        EffectWindow *w = transitionIt.key();
        const TimeLine &timeLine = transitionIt.value();
        if (timeLine.done()) {
            transitionIt = m_transitions.erase(transitionIt);
        } else {
            ++transitionIt;
        }
        w->addRepaintFull();
    }

    effects->postPaintScreen();
}

void DimInactiveEffect::dimWindow(WindowPaintData &data, qreal strength)
{
    qreal dimFactor = 1.0;
    if (m_fullScreenTransition.active) {
        dimFactor = 1.0 - m_fullScreenTransition.timeLine.value();
    } else if (effects->activeFullScreenEffect()) {
        dimFactor = 0.0;
    }

    data.multiplyBrightness(1.0 - strength * dimFactor);
    data.multiplySaturation(1.0 - strength * dimFactor);
}

bool DimInactiveEffect::canDimWindow(const EffectWindow *w) const
{
    if (m_activeWindow == w) {
        return false;
    }

    if (m_dimByGroup && belongToSameGroup(m_activeWindow, w)) {
        return false;
    }

    if (w->isDock() && !m_dimPanels) {
        return false;
    }

    if (w->isDesktop() && !m_dimDesktop) {
        return false;
    }

    if (w->keepAbove() && !m_dimKeepAbove) {
        return false;
    }

    if (!w->isManaged()) {
        return false;
    }

    return w->isNormalWindow()
        || w->isDialog()
        || w->isUtility()
        || w->isDock()
        || w->isDesktop();
}

void DimInactiveEffect::scheduleInTransition(EffectWindow *w)
{
    TimeLine &timeLine = m_transitions[w];
    timeLine.reset();
    timeLine.setDuration(
        std::chrono::milliseconds(static_cast<int>(animationTime(160))));
    timeLine.setDirection(TimeLine::Backward);
    timeLine.setEasingCurve(QEasingCurve::InCurve);
}

void DimInactiveEffect::scheduleGroupInTransition(EffectWindow *w)
{
    if (!m_dimByGroup) {
        scheduleInTransition(w);
        return;
    }

    if (!w->group()) {
        scheduleInTransition(w);
        return;
    }

    const auto members = w->group()->members();
    for (EffectWindow *member : members) {
        scheduleInTransition(member);
    }
}

void DimInactiveEffect::scheduleOutTransition(EffectWindow *w)
{
    TimeLine &timeLine = m_transitions[w];
    timeLine.reset();
    timeLine.setDuration(
        std::chrono::milliseconds(static_cast<int>(animationTime(250))));
    timeLine.setDirection(TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::OutCurve);
}

void DimInactiveEffect::scheduleGroupOutTransition(EffectWindow *w)
{
    if (!m_dimByGroup) {
        scheduleOutTransition(w);
        return;
    }

    if (!w->group()) {
        scheduleOutTransition(w);
        return;
    }

    const auto members = w->group()->members();
    for (EffectWindow *member : members) {
        scheduleOutTransition(member);
    }
}

void DimInactiveEffect::scheduleRepaint(EffectWindow *w)
{
    if (!m_dimByGroup) {
        w->addRepaintFull();
        return;
    }

    if (!w->group()) {
        w->addRepaintFull();
        return;
    }

    const auto members = w->group()->members();
    for (EffectWindow *member : members) {
        member->addRepaintFull();
    }
}

void DimInactiveEffect::windowActivated(EffectWindow *w)
{
    if (!w) {
        return;
    }

    if (m_activeWindow == w) {
        return;
    }

    if (m_dimByGroup && belongToSameGroup(m_activeWindow, w)) {
        m_activeWindow = w;
        return;
    }

    // WORKAROUND: Deleted windows do not belong to any of window groups.
    // So, if one of windows in a window group is closed, the In transition
    // will be false-triggered for the rest of the window group. In addition
    // to the active window, keep track of active window group so we can
    // tell whether "focus" moved from a closed window to some other window
    // in a window group.
    if (m_dimByGroup && w->group() && w->group() == m_activeWindowGroup) {
        m_activeWindow = w;
        return;
    }

    EffectWindow *previousActiveWindow = m_activeWindow;
    m_activeWindow = canDimWindow(w) ? w : nullptr;

    m_activeWindowGroup = (m_dimByGroup && m_activeWindow)
        ? m_activeWindow->group()
        : nullptr;

    if (previousActiveWindow) {
        scheduleGroupOutTransition(previousActiveWindow);
        scheduleRepaint(previousActiveWindow);
    }

    if (m_activeWindow) {
        scheduleGroupInTransition(m_activeWindow);
        scheduleRepaint(m_activeWindow);
    }
}

void DimInactiveEffect::windowClosed(EffectWindow *w)
{
    // When a window is closed, we should force current dim strength that
    // is applied to it to avoid flickering when some effect animates
    // the disappearing of the window. If there is no such effect then
    // it won't be dimmed.
    qreal forcedStrength = 0.0;
    bool shouldForceDim = false;

    auto transitionIt = m_transitions.find(w);
    if (transitionIt != m_transitions.end()) {
        forcedStrength = m_dimStrength * (*transitionIt).value();
        shouldForceDim = true;
        m_transitions.erase(transitionIt);
    } else if (m_activeWindow == w) {
        forcedStrength = 0.0;
        shouldForceDim = true;
    } else if (m_dimByGroup && belongToSameGroup(m_activeWindow, w)) {
        forcedStrength = 0.0;
        shouldForceDim = true;
    } else if (canDimWindow(w)) {
        forcedStrength = m_dimStrength;
        shouldForceDim = true;
    }

    if (shouldForceDim) {
        m_forceDim.insert(w, forcedStrength);
    }

    if (m_activeWindow == w) {
        m_activeWindow = nullptr;
    }
}

void DimInactiveEffect::windowDeleted(EffectWindow *w)
{
    m_forceDim.remove(w);
}

void DimInactiveEffect::activeFullScreenEffectChanged()
{
    m_fullScreenTransition.active = true;
    if (m_fullScreenTransition.timeLine.done()) {
        m_fullScreenTransition.timeLine.reset();
    }
    m_fullScreenTransition.timeLine.setDirection(
        effects->activeFullScreenEffect()
            ? TimeLine::Forward
            : TimeLine::Backward
    );

    effects->addRepaintFull();
}

} // namespace KWin
