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

// Qt
#include <QEasingCurve>

#include "slidedesktops.h"
// KConfigSkeleton
#include "slidedesktopsconfig.h"

namespace KWin
{

SlideDesktopsEffect::SlideDesktopsEffect()
{
    initConfig<SlideDesktopsConfig>();
    reconfigure(ReconfigureAll);
    connect(effects, static_cast<void (EffectsHandler::*)(int,int,EffectWindow*)>(&EffectsHandler::desktopChanged),
            this, &SlideDesktopsEffect::desktopChanged);
    connect(effects, &EffectsHandler::windowAdded,
            this, &SlideDesktopsEffect::windowAdded);
    connect(effects, &EffectsHandler::windowDeleted,
            this, &SlideDesktopsEffect::windowDeleted);
    connect(effects, &EffectsHandler::numberDesktopsChanged,
            this, &SlideDesktopsEffect::numberDesktopsChanged);
    connect(effects, &EffectsHandler::numberScreensChanged,
            this, &SlideDesktopsEffect::numberScreensChanged);
}

bool SlideDesktopsEffect::supported()
{
    return effects->animationsSupported();
}

void SlideDesktopsEffect::reconfigure(ReconfigureFlags)
{
    SlideDesktopsConfig::self()->read();

    const int d = animationTime(
        SlideDesktopsConfig::duration() != 0 ? SlideDesktopsConfig::duration() : 500);
    QEasingCurve easing(QEasingCurve::OutCubic);
    m_timeline.setDuration(d);
    m_timeline.setEasingCurve(easing);

    m_hGap = SlideDesktopsConfig::horizontalGap();
    m_vGap = SlideDesktopsConfig::verticalGap();
}

void SlideDesktopsEffect::prePaintScreen(ScreenPrePaintData& data, int time)
{
    if (m_active) {
        m_timeline.setCurrentTime(m_timeline.currentTime() + time);
        data.mask |= PAINT_SCREEN_TRANSFORMED
                  |  PAINT_SCREEN_BACKGROUND_FIRST;
    }

    effects->prePaintScreen(data, time);
}

/**
 * Wrap vector @p diff around grid @p w x @p h.
 *
 * Wrapping is done in such a way that magnitude of x and y component of vector
 * @p diff is less than half of @p w and half of @p h, respectively. This will
 * result in having the "shortest" path between two points.
 *
 * @param diff Vector between two points
 * @param w Width of the desktop grid
 * @param h Height of the desktop grid
 */
inline void wrapDiff(QPoint& diff, int w, int h)
{
    if (diff.x() > w/2) {
        diff.setX(diff.x() - w);
    } else if (diff.x() < -w/2) {
        diff.setX(diff.x() + w);
    }

    if (diff.y() > h/2) {
        diff.setY(diff.y() - h);
    } else if (diff.y() < -h/2) {
        diff.setY(diff.y() + h);
    }
}

inline QRegion buildClipRegion(QPoint pos, int w, int h)
{
    const QSize screenSize = effects->virtualScreenSize();
    QRegion r = QRect(pos, screenSize);
    if (effects->optionRollOverDesktops()) {
        r |= (r & QRect(-w, 0, w, h)).translated(w, 0);  // W
        r |= (r & QRect(w, 0, w, h)).translated(-w, 0);  // E

        r |= (r & QRect(0, -h, w, h)).translated(0, h);  // N
        r |= (r & QRect(0, h, w, h)).translated(0, -h);  // S

        r |= (r & QRect(-w, -h, w, h)).translated(w, h); // NW
        r |= (r & QRect(w, -h, w, h)).translated(-w, h); // NE
        r |= (r & QRect(w, h, w, h)).translated(-w, -h); // SE
        r |= (r & QRect(-w, h, w, h)).translated(w, -h); // SW
    }
    return r;
}

void SlideDesktopsEffect::paintScreen(int mask, QRegion region, ScreenPaintData& data)
{
    if (! m_active) {
        effects->paintScreen(mask, region, data);
        return;
    }

    const bool wrap = effects->optionRollOverDesktops();
    const int w = workspaceWidth();
    const int h = workspaceHeight();

    QPoint currentPos = m_startPos + m_diff * m_timeline.currentValue();

    // When "Desktop navigation wraps around" checkbox is checked, currentPos
    // can be outside the rectangle Rect{x:-w, y:-h, width:2*w, height: 2*h},
    // so we map currentPos back to the rect.
    if (wrap) {
        currentPos.setX(currentPos.x() % w);
        currentPos.setY(currentPos.y() % h);
    }

    QRegion clipRegion = buildClipRegion(currentPos, w, h);
    QList<int> visibleDesktops;
    for (int i = 1; i <= effects->numberOfDesktops(); i++) {
        QRect desktopGeo = desktopGeometry(i);
        if (! clipRegion.contains(desktopGeo)) {
            continue;
        }
        visibleDesktops << i;
    }

    // When we enter a virtual desktop that has a window in fullscreen mode,
    // stacking order is fine. When we leave a virtual desktop that has
    // a window in fullscreen mode, stacking order is no longer valid
    // because panels are raised above the fullscreen window. Construct
    // a list of fullscreen windows, so we can decide later whether
    // docks should be visible on different virtual desktops.
    m_paintCtx.fullscreenWindows.clear();
    foreach (EffectWindow* w, effects->stackingOrder()) {
        if (! w->isFullScreen()) {
            continue;
        }
        m_paintCtx.fullscreenWindows << w;
    }

    m_paintCtx.firstPass = true;
    const int lastDesktop = visibleDesktops.last();
    foreach (int desktop, visibleDesktops) {
        m_paintCtx.desktop = desktop;
        m_paintCtx.lastPass = (lastDesktop == desktop);
        m_paintCtx.translation = desktopCoords(desktop) - currentPos;
        if (wrap) {
            wrapDiff(m_paintCtx.translation, w, h);
        }
        effects->paintScreen(mask, region, data);
        m_paintCtx.firstPass = false;
    }
}

/**
 * Decide whether given window @p w should be transformed/translated.
 * @returns @c true if given window @p w should be transformed, otherwise @c false
 */
bool SlideDesktopsEffect::isTranslated(const EffectWindow* w) const
{
    if (w->isOnAllDesktops()) {
        return (w->isDock() || w->isDesktop());
    } else if (w == m_movingWindow) {
        return false;
    } else if (w->isOnDesktop(m_paintCtx.desktop)) {
        return true;
    }
    return false;
}

/**
 * Decide whether given window @p w should be painted.
 * @returns @c true if given window @p w should be painted, otherwise @c false
 */
bool SlideDesktopsEffect::isPainted(const EffectWindow* w) const
{
    if (w->isOnAllDesktops()) {
        // In order to make sure that 'keep above' windows are above
        // other windows during transition to another virtual desktop,
        // they should be painted in the last pass.
        if (w->keepAbove()) {
            return m_paintCtx.lastPass;
        }
        // Do not draw dock windows if there is a fullscreen window
        // on currently painted desktop.
        if (w->isDock()) {
            foreach (EffectWindow* fw, m_paintCtx.fullscreenWindows) {
                if (fw->isOnDesktop(m_paintCtx.desktop)
                    && fw->screen() == w->screen()) {
                    return false;
                }
            }
        }
        return true;
    } if (w == m_movingWindow) {
        return m_paintCtx.lastPass;
    } else if (w->isOnDesktop(m_paintCtx.desktop)) {
        return true;
    }
    return false;
}

void SlideDesktopsEffect::prePaintWindow(EffectWindow* w, WindowPrePaintData& data, int time)
{
    if (m_active) {
        const bool painted = isPainted(w);
        if (painted) {
            w->enablePainting(EffectWindow::PAINT_DISABLED_BY_DESKTOP);
        } else {
            w->disablePainting(EffectWindow::PAINT_DISABLED_BY_DESKTOP);
        }
        if (painted && isTranslated(w)) {
            data.setTransformed();
        }
    }
    effects->prePaintWindow(w, data, time);
}

void SlideDesktopsEffect::paintWindow(EffectWindow* w, int mask, QRegion region, WindowPaintData& data)
{
    if (m_active && isTranslated(w)) {
        data += m_paintCtx.translation;
    }
    effects->paintWindow(w, mask, region, data);
}

void SlideDesktopsEffect::postPaintScreen()
{
    if (m_active) {
        if (m_timeline.currentValue() == 1) {
            stop();
        }
        effects->addRepaintFull();
    }
    effects->postPaintScreen();
}

/**
 * Get position of the top-left corner of desktop @p id within desktop grid with gaps.
 * @param id ID of a virtual desktop
 */
QPoint SlideDesktopsEffect::desktopCoords(int id) const
{
    QPoint c = effects->desktopCoords(id);
    QPoint gridPos = effects->desktopGridCoords(id);
    c.setX(c.x() + m_hGap * gridPos.x());
    c.setY(c.y() + m_vGap * gridPos.y());
    return c;
}

/**
 * Get geometry of desktop @p id within desktop grid with gaps.
 * @param id ID of a virtual desktop
 */
QRect SlideDesktopsEffect::desktopGeometry(int id) const
{
    QRect g = effects->virtualScreenGeometry();
    g.translate(desktopCoords(id));
    return g;
}

/**
 * Get width of a virtual desktop grid.
 */
int SlideDesktopsEffect::workspaceWidth() const
{
    int w = effects->workspaceWidth();
    w += m_hGap * effects->desktopGridWidth();
    return w;
}

/**
 * Get height of a virtual desktop grid.
 */
int SlideDesktopsEffect::workspaceHeight() const
{
    int h = effects->workspaceHeight();
    h += m_vGap * effects->desktopGridHeight();
    return h;
}

inline bool shouldForceBackgroundContrast(const EffectWindow *w)
{
    if (w->data(WindowForceBackgroundContrastRole).isValid()) {
        return false;
    }
    return w->hasAlpha()
        && w->isOnAllDesktops()
        && (w->isDock() || w->keepAbove());
}

void SlideDesktopsEffect::start(int old, int current, EffectWindow* movingWindow)
{
    m_movingWindow = movingWindow;

    const bool wrap = effects->optionRollOverDesktops();
    const int w = workspaceWidth();
    const int h = workspaceHeight();

    if (m_active) {
        QPoint passed = m_diff * m_timeline.currentValue();
        QPoint currentPos = m_startPos + passed;
        QPoint delta = desktopCoords(current) - desktopCoords(old);
        if (wrap) {
            wrapDiff(delta, w, h);
        }
        m_diff += delta - passed;
        m_startPos = currentPos;
        m_timeline.setCurrentTime(0);
        return;
    }

    foreach (EffectWindow* w, effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant(true));
        if (shouldForceBackgroundContrast(w)) {
            w->setData(WindowForceBackgroundContrastRole, QVariant(true));
            m_backgroundContrastForcedBefore << w;
        }
    }

    m_diff = desktopCoords(current) - desktopCoords(old);
    if (wrap) {
        wrapDiff(m_diff, w, h);
    }
    m_startPos = desktopCoords(old);
    m_timeline.setCurrentTime(0);
    m_active = true;
    effects->setActiveFullScreenEffect(this);
    effects->addRepaintFull();
}

void SlideDesktopsEffect::stop()
{
    foreach (EffectWindow* w, effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant(false));
        if (m_backgroundContrastForcedBefore.contains(w)) {
            w->setData(WindowForceBackgroundContrastRole, QVariant());
        }
    }
    m_backgroundContrastForcedBefore.clear();
    m_paintCtx.fullscreenWindows.clear();
    m_timeline.setCurrentTime(0);
    m_movingWindow = nullptr;
    m_active = false;
    effects->setActiveFullScreenEffect(nullptr);
}

void SlideDesktopsEffect::desktopChanged(int old, int current, EffectWindow* with)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this) {
        return;
    }
    start(old, current, with);
}

void SlideDesktopsEffect::windowAdded(EffectWindow *w)
{
    if (! m_active) {
        return;
    }
    w->setData(WindowForceBlurRole, QVariant(true));
    if (shouldForceBackgroundContrast(w)) {
        w->setData(WindowForceBackgroundContrastRole, QVariant(true));
        m_backgroundContrastForcedBefore << w;
    }
}

void SlideDesktopsEffect::windowDeleted(EffectWindow *w)
{
    if (! m_active) {
        return;
    }
    m_backgroundContrastForcedBefore.removeAll(w);
    m_paintCtx.fullscreenWindows.removeAll(w);
    if (w == m_movingWindow) {
        m_movingWindow = nullptr;
    }
}

void SlideDesktopsEffect::numberDesktopsChanged(uint)
{
    if (! m_active) {
        return;
    }
    stop();
}

void SlideDesktopsEffect::numberScreensChanged()
{
    if (! m_active) {
        return;
    }
    stop();
}

} // namespace
