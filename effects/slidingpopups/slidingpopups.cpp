/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2009 Marco Martin notmart@gmail.com

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
#include "slidingpopups.h"

// KConfigSkeleton
#include "slidingpopupsconfig.h"

// KWayland
#include <KWayland/Server/display.h>
#include <KWayland/Server/slide_interface.h>
#include <KWayland/Server/surface_interface.h>

// Qt
#include <QApplication>
#include <QFontMetrics>

namespace KWin
{

SlidingPopupsEffect::SlidingPopupsEffect()
{
    initConfig<SlidingPopupsConfig>();
    reconfigure(ReconfigureAll);

    KWayland::Server::Display *display = effects->waylandDisplay();
    if (display) {
        display->createSlideManager(this)->create();
    }

    m_slideLength = QFontMetrics(qApp->font()).height() * 8;

    m_atom = effects->announceSupportProperty(QByteArrayLiteral("_KDE_SLIDE"), this);
    connect(effects, &EffectsHandler::windowAdded, this, &SlidingPopupsEffect::windowAdded);
    connect(effects, &EffectsHandler::windowClosed, this, &SlidingPopupsEffect::slideOut);
    connect(effects, &EffectsHandler::windowDeleted, this, &SlidingPopupsEffect::windowDeleted);
    connect(effects, &EffectsHandler::propertyNotify, this, &SlidingPopupsEffect::propertyNotify);
    connect(effects, &EffectsHandler::windowShown, this, &SlidingPopupsEffect::slideIn);
    connect(effects, &EffectsHandler::windowHidden, this, &SlidingPopupsEffect::slideOut);
    connect(effects, &EffectsHandler::xcbConnectionChanged, this,
        [this] {
            m_atom = effects->announceSupportProperty(QByteArrayLiteral("_KDE_SLIDE"), this);
        }
    );
}

SlidingPopupsEffect::~SlidingPopupsEffect()
{
}

void SlidingPopupsEffect::reconfigure(ReconfigureFlags flags)
{
    Q_UNUSED(flags)

    SlidingPopupsConfig::self()->read();

    m_slideInDuration = std::chrono::milliseconds(
        static_cast<int>(animationTime(SlidingPopupsConfig::slideInTime() != 0 ? SlidingPopupsConfig::slideInTime() : 150)));
    m_slideOutDuration = std::chrono::milliseconds(
        static_cast<int>(animationTime(SlidingPopupsConfig::slideOutTime() != 0 ? SlidingPopupsConfig::slideOutTime() : 250)));

    auto animationIt = m_animations.begin();
    while (animationIt != m_animations.end()) {
        const auto duration = (*animationIt).kind == AnimationKind::In
            ? m_slideInDuration
            : m_slideOutDuration;
        (*animationIt).timeLine.setDuration(duration);
        ++animationIt;
    }

    auto dataIt = m_animationData.begin();
    while (dataIt != m_animationData.end()) {
        (*dataIt).slideInDuration = m_slideInDuration;
        (*dataIt).slideOutDuration = m_slideOutDuration;
        ++dataIt;
    }
}

void SlidingPopupsEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, int time)
{
    auto animationIt = m_animations.find(w);
    if (animationIt == m_animations.end()) {
        effects->prePaintWindow(w, data, time);
        return;
    }

    (*animationIt).timeLine.update(std::chrono::milliseconds(time));

    data.setTransformed();
    w->enablePainting(EffectWindow::PAINT_DISABLED | EffectWindow::PAINT_DISABLED_BY_DELETE);

    const AnimationData &animData = m_animationData[w];
    if (animData.offset != 0) {
        const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), effects->currentDesktop());
        const QRect geo = w->expandedGeometry();
        const qreal t = (*animationIt).timeLine.value();

        const int slideLength = animData.slideLength > 0 ? animData.slideLength : m_slideLength;

        switch (animData.location) {
        case Location::Left: {
            const qreal splitPoint = geo.width() - (geo.x() + geo.width() - screenRect.x() - animData.offset) + interpolate(qMin(geo.width(), slideLength), 0.0, t);
            data.quads = data.quads.splitAtX(splitPoint);
            WindowQuadList filtered;
            filtered.reserve(data.quads.count());
            for (const WindowQuad &quad : qAsConst(data.quads)) {
                if (quad.left() >= splitPoint) {
                    filtered << quad;
                }
            }
            data.quads = filtered;
            break;
        }
        case Location::Top: {
            const qreal splitPoint = geo.height() - (geo.y() + geo.height() - screenRect.y() - animData.offset) + interpolate(qMin(geo.height(), slideLength), 0.0, t);
            data.quads = data.quads.splitAtY(splitPoint);
            WindowQuadList filtered;
            filtered.reserve(data.quads.count());
            for (const WindowQuad &quad : qAsConst(data.quads)) {
                if (quad.top() >= splitPoint) {
                    filtered << quad;
                }
            }
            data.quads = filtered;
            break;
        }
        case Location::Right: {
            const qreal splitPoint = screenRect.x() + screenRect.width() - geo.x() - animData.offset - interpolate(qMin(geo.width(), slideLength), 0.0, t);
            data.quads = data.quads.splitAtX(splitPoint);
            WindowQuadList filtered;
            filtered.reserve(data.quads.count());
            for (const WindowQuad &quad : qAsConst(data.quads)) {
                if (quad.right() <= splitPoint) {
                    filtered << quad;
                }
            }
            data.quads = filtered;
            break;
        }
        case Location::Bottom:
        default: {
            const qreal splitPoint = screenRect.y() + screenRect.height() - geo.y() - animData.offset - interpolate(qMin(geo.height(), slideLength), 0.0, t);
            data.quads = data.quads.splitAtY(splitPoint);
            WindowQuadList filtered;
            filtered.reserve(data.quads.count());
            for (const WindowQuad &quad : qAsConst(data.quads)) {
                if (quad.bottom() <= splitPoint) {
                    filtered << quad;
                }
            }
            data.quads = filtered;
            break;
        }
        }
    }

    effects->prePaintWindow(w, data, time);
}

void SlidingPopupsEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    auto animationIt = m_animations.constFind(w);
    if (animationIt == m_animations.constEnd()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    const AnimationData &animData = m_animationData[w];
    const int slideLength = animData.slideLength > 0 ? animData.slideLength : m_slideLength;

    const qreal t = (*animationIt).timeLine.value();

    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), w->desktop());
    const QRect geo = w->expandedGeometry();

    int splitPoint = 0;
    switch(animData.location) {
    case Location::Left:
        if (slideLength < geo.width()) {
            data.multiplyOpacity(t);
        }
        data.translate(-interpolate(qMin(geo.width(), slideLength), 0.0, t));
        splitPoint = geo.width() - (geo.x() + geo.width() - screenRect.x() - animData.offset);
        region = QRegion(geo.x() + splitPoint, geo.y(), geo.width() - splitPoint, geo.height());
        break;
    case Location::Top:
        if (slideLength < geo.height()) {
            data.multiplyOpacity(t);
        }
        data.translate(0.0, -interpolate(qMin(geo.height(), slideLength), 0.0, t));
        splitPoint = geo.height() - (geo.y() + geo.height() - screenRect.y() - animData.offset);
        region = QRegion(geo.x(), geo.y() + splitPoint, geo.width(), geo.height() - splitPoint);
        break;
    case Location::Right:
        if (slideLength < geo.width()) {
            data.multiplyOpacity(t);
        }
        data.translate(interpolate(qMin(geo.width(), slideLength), 0.0, t));
        splitPoint = screenRect.x() + screenRect.width() - geo.x() - animData.offset;
        region = QRegion(geo.x(), geo.y(), splitPoint, geo.height());
        break;
    case Location::Bottom:
    default:
        if (slideLength < geo.height()) {
            data.multiplyOpacity(t);
        }
        data.translate(0.0, interpolate(qMin(geo.height(), slideLength), 0.0, t));
        splitPoint = screenRect.y() + screenRect.height() - geo.y() - animData.offset;
        region = QRegion(geo.x(), geo.y(), geo.width(), splitPoint);
    }

    effects->paintWindow(w, mask, region, data);
}

void SlidingPopupsEffect::postPaintWindow(EffectWindow *w)
{
    auto animationIt = m_animations.find(w);
    if (animationIt != m_animations.end()) {
        if ((*animationIt).timeLine.done()) {
            if (w->isDeleted()) {
                w->unrefWindow();
            } else {
                w->setData(WindowForceBlurRole, QVariant());
                w->setData(WindowForceBackgroundContrastRole, QVariant());
            }
            m_animations.erase(animationIt);
        }
        w->addRepaintFull();
    }

    effects->postPaintWindow(w);
}

bool SlidingPopupsEffect::isActive() const
{
    return !m_animations.isEmpty();
}

bool SlidingPopupsEffect::supported()
{
     return effects->animationsSupported()
         && effects->isOpenGLCompositing();
}

void SlidingPopupsEffect::windowAdded(EffectWindow *w)
{
    // X11
    propertyNotify(w, m_atom);

    // Wayland
    if (auto *surf = w->surface()) {
        waylandSlideOnShowChanged(w);
        connect(surf, &KWayland::Server::SurfaceInterface::slideOnShowHideChanged, this, [this, surf] {
            waylandSlideOnShowChanged(effects->findWindow(surf));
        });
    }

    slideIn(w);
}

void SlidingPopupsEffect::windowDeleted(EffectWindow *w)
{
    m_animations.remove(w);
    m_animationData.remove(w);
    effects->addRepaint(w->expandedGeometry());
}

void SlidingPopupsEffect::propertyNotify(EffectWindow *w, long atom)
{
    if (!w) {
        return;
    }

    if (atom != m_atom || m_atom == XCB_ATOM_NONE) {
        return;
    }

    const QByteArray data = w->readProperty(m_atom, m_atom, 32);

    if (data.isEmpty()) {
        // Property was removed, thus also remove the effect for the window.
        if (w->data(WindowClosedGrabRole).value<void *>() == this) {
            w->setData(WindowClosedGrabRole, QVariant());
        }
        m_animations.remove(w);
        m_animationData.remove(w);
        return;
    }

    const auto *d = reinterpret_cast<const uint32_t*>(data.data());
    AnimationData &animData = m_animationData[w];
    animData.offset = d[0];

    switch (d[1]) {
    case 0:
        animData.location = Location::Left;
        break;

    case 1:
        animData.location = Location::Top;
        break;

    case 2:
        animData.location = Location::Right;
        break;

    case 3:
    default:
        animData.location = Location::Bottom;
        break;
    }

    // Custom duration.
    animData.slideLength = 0;
    if (static_cast<size_t>(data.length()) >= sizeof(uint32_t) * 3) {
        animData.slideInDuration = std::chrono::milliseconds(d[2]);

        if (static_cast<size_t>(data.length()) >= sizeof(uint32_t) * 4) {
            animData.slideOutDuration = std::chrono::milliseconds(d[3]);
        } else {
            animData.slideOutDuration = animData.slideInDuration;
        }

        if (static_cast<size_t>(data.length()) >= sizeof(uint32_t) * 5) {
            animData.slideLength = d[4];
        }
    } else {
        animData.slideInDuration = m_slideInDuration;
        animData.slideOutDuration = m_slideOutDuration;
    }

    sanitizeAnimData(w);
}

void SlidingPopupsEffect::waylandSlideOnShowChanged(EffectWindow *w)
{
    if (!w) {
        return;
    }

    KWayland::Server::SurfaceInterface *surf = w->surface();
    if (!surf || !surf->slideOnShowHide()) {
        return;
    }

    AnimationData &animData = m_animationData[w];
    animData.offset = surf->slideOnShowHide()->offset();

    switch (surf->slideOnShowHide()->location()) {
    case KWayland::Server::SlideInterface::Location::Top:
        animData.location = Location::Top;
        break;
    case KWayland::Server::SlideInterface::Location::Left:
        animData.location = Location::Left;
        break;
    case KWayland::Server::SlideInterface::Location::Right:
        animData.location = Location::Right;
        break;
    case KWayland::Server::SlideInterface::Location::Bottom:
    default:
        animData.location = Location::Bottom;
        break;
    }

    animData.slideLength = 0;
    animData.slideInDuration = m_slideInDuration;
    animData.slideOutDuration = m_slideOutDuration;

    sanitizeAnimData(w);
}

void SlidingPopupsEffect::sanitizeAnimData(EffectWindow *w)
{
    const QRect screenRect = effects->clientArea(FullScreenArea, w->screen(), effects->currentDesktop());
    const QRect windowGeo = w->geometry();
    AnimationData &animData = m_animationData[w];

    if (animData.offset == -1) {
        switch (animData.location) {
        case Location::Left:
            animData.offset = qMax(windowGeo.left() - screenRect.left(), 0);
            break;
        case Location::Top:
            animData.offset = qMax(windowGeo.top() - screenRect.top(), 0);
            break;
        case Location::Right:
            animData.offset = qMax(screenRect.right() - windowGeo.right(), 0);
            break;
        case Location::Bottom:
        default:
            animData.offset = qMax(screenRect.bottom() - windowGeo.bottom(), 0);
            break;
        }
        return;
    }

    switch (animData.location) {
    case Location::Left:
        animData.offset = qMax(windowGeo.left() - screenRect.left(), animData.offset);
        break;
    case Location::Top:
        animData.offset = qMax(windowGeo.top() - screenRect.top(), animData.offset);
        break;
    case Location::Right:
        animData.offset = qMax(screenRect.right() - windowGeo.right(), animData.offset);
        break;
    case Location::Bottom:
    default:
        animData.offset = qMax(screenRect.bottom() - windowGeo.bottom(), animData.offset);
        break;
    }
}

void SlidingPopupsEffect::slideIn(EffectWindow *w)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    if (!w->isVisible()) {
        return;
    }

    auto dataIt = m_animationData.constFind(w);
    if (dataIt == m_animationData.constEnd()) {
        return;
    }

    Animation &animation = m_animations[w];
    animation.kind = AnimationKind::In;
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Forward);
    animation.timeLine.setDuration((*dataIt).slideInDuration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    w->setData(WindowAddedGrabRole, QVariant::fromValue(static_cast<void*>(this)));
    w->setData(WindowForceBackgroundContrastRole, QVariant(true));
    w->setData(WindowForceBlurRole, QVariant(true));

    w->addRepaintFull();
}

void SlidingPopupsEffect::slideOut(EffectWindow *w)
{
    if (effects->activeFullScreenEffect()) {
        return;
    }

    if (!w->isVisible()) {
        return;
    }

    auto dataIt = m_animationData.constFind(w);
    if (dataIt == m_animationData.constEnd()) {
        return;
    }

    if (w->isDeleted()) {
        w->refWindow();
    }

    Animation &animation = m_animations[w];
    animation.kind = AnimationKind::Out;
    animation.timeLine.reset();
    animation.timeLine.setDirection(TimeLine::Backward);
    animation.timeLine.setDuration((*dataIt).slideOutDuration);
    animation.timeLine.setEasingCurve(QEasingCurve::InOutSine);

    w->setData(WindowClosedGrabRole, QVariant::fromValue(static_cast<void*>(this)));
    w->setData(WindowForceBackgroundContrastRole, QVariant(true));
    w->setData(WindowForceBlurRole, QVariant(true));

    w->addRepaintFull();
}

} // namespace KWin
