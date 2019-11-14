/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

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

/*
 The base class for compositing, implementing shared functionality
 between the OpenGL and XRender backends.

 Design:

 When compositing is turned on, XComposite extension is used to redirect
 drawing of windows to pixmaps and XDamage extension is used to get informed
 about damage (changes) to window contents. This code is mostly in composite.cpp .

 Compositor::performCompositing() starts one painting pass. Painting is done
 by painting the screen, which in turn paints every window. Painting can be affected
 using effects, which are chained. E.g. painting a screen means that actually
 paintScreen() of the first effect is called, which possibly does modifications
 and calls next effect's paintScreen() and so on, until Scene::finalPaintScreen()
 is called.

 There are 3 phases of every paint (not necessarily done together):
 The pre-paint phase, the paint phase and the post-paint phase.

 The pre-paint phase is used to find out about how the painting will be actually
 done (i.e. what the effects will do). For example when only a part of the screen
 needs to be updated and no effect will do any transformation it is possible to use
 an optimized paint function. How the painting will be done is controlled
 by the mask argument, see PAINT_WINDOW_* and PAINT_SCREEN_* flags in scene.h .
 For example an effect that decides to paint a normal windows as translucent
 will need to modify the mask in its prePaintWindow() to include
 the PAINT_WINDOW_TRANSLUCENT flag. The paintWindow() function will then get
 the mask with this flag turned on and will also paint using transparency.

 The paint pass does the actual painting, based on the information collected
 using the pre-paint pass. After running through the effects' paintScreen()
 either paintGenericScreen() or optimized paintSimpleScreen() are called.
 Those call paintWindow() on windows (not necessarily all), possibly using
 clipping to optimize performance and calling paintWindow() first with only
 PAINT_WINDOW_OPAQUE to paint the opaque parts and then later
 with PAINT_WINDOW_TRANSLUCENT to paint the transparent parts. Function
 paintWindow() again goes through effects' paintWindow() until
 finalPaintWindow() is called, which calls the window's performPaint() to
 do the actual painting.

 The post-paint can be used for cleanups and is also used for scheduling
 repaints during the next painting pass for animations. Effects wanting to
 repaint certain parts can manually damage them during post-paint and repaint
 of these parts will be done during the next paint pass.

*/

#include "scene.h"

#include <QQuickWindow>
#include <QVector2D>

#include "x11client.h"
#include "deleted.h"
#include "effects.h"
#include "overlaywindow.h"
#include "screens.h"
#include "shadow.h"
#include "wayland_server.h"

#include "thumbnailitem.h"

#include <KWayland/Server/buffer_interface.h>
#include <KWayland/Server/subcompositor_interface.h>
#include <KWayland/Server/surface_interface.h>

namespace KWin
{

//****************************************
// Scene
//****************************************

Scene::Scene(QObject *parent)
    : QObject(parent)
{
    last_time.invalidate(); // Initialize the timer
}

Scene::~Scene()
{
    Q_ASSERT(m_windows.isEmpty());
}

// returns mask and possibly modified region
void Scene::paintScreen(int* mask, const QRegion &damage, const QRegion &repaint,
                        QRegion *updateRegion, QRegion *validRegion, const QMatrix4x4 &projection, const QRect &outputGeometry)
{
    const QSize &screenSize = screens()->size();
    const QRegion displayRegion(0, 0, screenSize.width(), screenSize.height());
    *mask = (damage == displayRegion) ? 0 : PAINT_SCREEN_REGION;

    updateTimeDiff();
    // preparation step
    static_cast<EffectsHandlerImpl*>(effects)->startPaint();

    QRegion region = damage;

    ScreenPrePaintData pdata;
    pdata.mask = *mask;
    pdata.paint = region;

    effects->prePaintScreen(pdata, time_diff);
    *mask = pdata.mask;
    region = pdata.paint;

    if (*mask & (PAINT_SCREEN_TRANSFORMED | PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS)) {
        // Region painting is not possible with transformations,
        // because screen damage doesn't match transformed positions.
        *mask &= ~PAINT_SCREEN_REGION;
        region = infiniteRegion();
    } else if (*mask & PAINT_SCREEN_REGION) {
        // make sure not to go outside visible screen
        region &= displayRegion;
    } else {
        // whole screen, not transformed, force region to be full
        region = displayRegion;
    }

    painted_region = region;
    repaint_region = repaint;

    if (*mask & PAINT_SCREEN_BACKGROUND_FIRST) {
        paintBackground(region);
    }

    ScreenPaintData data(projection, outputGeometry);
    effects->paintScreen(*mask, region, data);

    foreach (Window *w, stacking_order) {
        effects->postPaintWindow(effectWindow(w));
    }

    effects->postPaintScreen();

    // make sure not to go outside of the screen area
    *updateRegion = damaged_region;
    *validRegion = (region | painted_region) & displayRegion;

    repaint_region = QRegion();
    damaged_region = QRegion();

    // make sure all clipping is restored
    Q_ASSERT(!PaintClipper::clip());
}

// Compute time since the last painting pass.
void Scene::updateTimeDiff()
{
    if (!last_time.isValid()) {
        // Painting has been idle (optimized out) for some time,
        // which means time_diff would be huge and would break animations.
        // Simply set it to one (zero would mean no change at all and could
        // cause problems).
        time_diff = 1;
        last_time.start();
    } else

    time_diff = last_time.restart();

    if (time_diff < 0)   // check time rollback
        time_diff = 1;
}

// Painting pass is optimized away.
void Scene::idle()
{
    // Don't break time since last paint for the next pass.
    last_time.invalidate();
}

// the function that'll be eventually called by paintScreen() above
void Scene::finalPaintScreen(int mask, QRegion region, ScreenPaintData& data)
{
    if (mask & (PAINT_SCREEN_TRANSFORMED | PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS))
        paintGenericScreen(mask, data);
    else
        paintSimpleScreen(mask, region);
}

// The generic painting code that can handle even transformations.
// It simply paints bottom-to-top.
void Scene::paintGenericScreen(int orig_mask, ScreenPaintData)
{
    if (!(orig_mask & PAINT_SCREEN_BACKGROUND_FIRST)) {
        paintBackground(infiniteRegion());
    }
    QVector<Phase2Data> phase2;
    phase2.reserve(stacking_order.size());
    foreach (Window * w, stacking_order) { // bottom to top
        Toplevel* topw = w->toplevel();

        // Reset the repaint_region.
        // This has to be done here because many effects schedule a repaint for
        // the next frame within Effects::prePaintWindow.
        topw->resetRepaints();

        WindowPrePaintData data;
        data.mask = orig_mask | (w->isOpaque() ? PAINT_WINDOW_OPAQUE : PAINT_WINDOW_TRANSLUCENT);
        w->resetPaintingEnabled();
        data.paint = infiniteRegion(); // no clipping, so doesn't really matter
        data.clip = QRegion();
        data.quads = w->buildQuads();
        // preparation step
        effects->prePaintWindow(effectWindow(w), data, time_diff);
#if !defined(QT_NO_DEBUG)
        if (data.quads.isTransformed()) {
            qFatal("Pre-paint calls are not allowed to transform quads!");
        }
#endif
        if (!w->isPaintingEnabled()) {
            continue;
        }
        phase2.append({w, infiniteRegion(), data.clip, data.mask, data.quads});
    }

    foreach (const Phase2Data & d, phase2) {
        paintWindow(d.window, d.mask, d.region, d.quads);
    }

    const QSize &screenSize = screens()->size();
    damaged_region = QRegion(0, 0, screenSize.width(), screenSize.height());
}

// The optimized case without any transformations at all.
// It can paint only the requested region and can use clipping
// to reduce painting and improve performance.
void Scene::paintSimpleScreen(int orig_mask, QRegion region)
{
    Q_ASSERT((orig_mask & (PAINT_SCREEN_TRANSFORMED
                         | PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS)) == 0);
    QVector<Phase2Data> phase2data;
    phase2data.reserve(stacking_order.size());

    QRegion dirtyArea = region;
    bool opaqueFullscreen(false);
    for (int i = 0;  // do prePaintWindow bottom to top
            i < stacking_order.count();
            ++i) {
        Window* w = stacking_order[ i ];
        Toplevel* topw = w->toplevel();
        WindowPrePaintData data;
        data.mask = orig_mask | (w->isOpaque() ? PAINT_WINDOW_OPAQUE : PAINT_WINDOW_TRANSLUCENT);
        w->resetPaintingEnabled();
        data.paint = region;
        data.paint |= topw->repaints();

        // Reset the repaint_region.
        // This has to be done here because many effects schedule a repaint for
        // the next frame within Effects::prePaintWindow.
        topw->resetRepaints();

        // Clip out the decoration for opaque windows; the decoration is drawn in the second pass
        opaqueFullscreen = false; // TODO: do we care about unmanged windows here (maybe input windows?)
        if (w->isOpaque()) {
            AbstractClient *c = dynamic_cast<AbstractClient*>(topw);
            if (c) {
                opaqueFullscreen = c->isFullScreen();
            }
            X11Client *cc = dynamic_cast<X11Client *>(c);
            // the window is fully opaque
            if (cc && cc->decorationHasAlpha()) {
                // decoration uses alpha channel, so we may not exclude it in clipping
                data.clip = w->clientShape().translated(w->x(), w->y());
            } else {
                // decoration is fully opaque
                if (c && c->isShade()) {
                    data.clip = QRegion();
                } else {
                    data.clip = w->shape().translated(w->x(), w->y());
                }
            }
        } else if (topw->hasAlpha() && topw->opacity() == 1.0) {
            // the window is partially opaque
            data.clip = (w->clientShape() & topw->opaqueRegion().translated(topw->clientPos())).translated(w->x(), w->y());
        } else {
            data.clip = QRegion();
        }
        data.quads = w->buildQuads();
        // preparation step
        effects->prePaintWindow(effectWindow(w), data, time_diff);
#if !defined(QT_NO_DEBUG)
        if (data.quads.isTransformed()) {
            qFatal("Pre-paint calls are not allowed to transform quads!");
        }
#endif
        if (!w->isPaintingEnabled()) {
            continue;
        }
        dirtyArea |= data.paint;
        // Schedule the window for painting
        phase2data.append({w, data.paint, data.clip, data.mask, data.quads});
    }

    // Save the part of the repaint region that's exclusively rendered to
    // bring a reused back buffer up to date. Then union the dirty region
    // with the repaint region.
    const QRegion repaintClip = repaint_region - dirtyArea;
    dirtyArea |= repaint_region;

    const QSize &screenSize = screens()->size();
    const QRegion displayRegion(0, 0, screenSize.width(), screenSize.height());
    bool fullRepaint(dirtyArea == displayRegion); // spare some expensive region operations
    if (!fullRepaint) {
        extendPaintRegion(dirtyArea, opaqueFullscreen);
        fullRepaint = (dirtyArea == displayRegion);
    }

    QRegion allclips, upperTranslucentDamage;
    upperTranslucentDamage = repaint_region;

    // This is the occlusion culling pass
    for (int i = phase2data.count() - 1; i >= 0; --i) {
        Phase2Data *data = &phase2data[i];

        if (fullRepaint)
            data->region = displayRegion;
        else
            data->region |= upperTranslucentDamage;

        // subtract the parts which will possibly been drawn as part of
        // a higher opaque window
        data->region -= allclips;

        // Here we rely on WindowPrePaintData::setTranslucent() to remove
        // the clip if needed.
        if (!data->clip.isEmpty() && !(data->mask & PAINT_WINDOW_TRANSLUCENT)) {
            // clip away the opaque regions for all windows below this one
            allclips |= data->clip;
            // extend the translucent damage for windows below this by remaining (translucent) regions
            if (!fullRepaint)
                upperTranslucentDamage |= data->region - data->clip;
        } else if (!fullRepaint) {
            upperTranslucentDamage |= data->region;
        }
    }

    QRegion paintedArea;
    // Fill any areas of the root window not covered by opaque windows
    if (!(orig_mask & PAINT_SCREEN_BACKGROUND_FIRST)) {
        paintedArea = dirtyArea - allclips;
        paintBackground(paintedArea);
    }

    // Now walk the list bottom to top and draw the windows.
    for (int i = 0; i < phase2data.count(); ++i) {
        Phase2Data *data = &phase2data[i];

        // add all regions which have been drawn so far
        paintedArea |= data->region;
        data->region = paintedArea;

        paintWindow(data->window, data->mask, data->region, data->quads);
    }

    if (fullRepaint) {
        painted_region = displayRegion;
        damaged_region = displayRegion;
    } else {
        painted_region |= paintedArea;

        // Clip the repainted region from the damaged region.
        // It's important that we don't add the union of the damaged region
        // and the repainted region to the damage history. Otherwise the
        // repaint region will grow with every frame until it eventually
        // covers the whole back buffer, at which point we're always doing
        // full repaints.
        damaged_region = paintedArea - repaintClip;
    }
}

void Scene::addToplevel(Toplevel *c)
{
    Q_ASSERT(!m_windows.contains(c));
    Scene::Window *w = createWindow(c);
    m_windows[ c ] = w;
    connect(c, SIGNAL(geometryShapeChanged(KWin::Toplevel*,QRect)), SLOT(windowGeometryShapeChanged(KWin::Toplevel*)));
    connect(c, SIGNAL(windowClosed(KWin::Toplevel*,KWin::Deleted*)), SLOT(windowClosed(KWin::Toplevel*,KWin::Deleted*)));
    //A change of scale won't affect the geometry in compositor co-ordinates, but will affect the window quads.
    if (c->surface()) {
        connect(c->surface(), &KWayland::Server::SurfaceInterface::scaleChanged, this, std::bind(&Scene::windowGeometryShapeChanged, this, c));
    }
    connect(c, &Toplevel::screenScaleChanged, this,
        [this, c] {
            windowGeometryShapeChanged(c);
        }
    );
    c->effectWindow()->setSceneWindow(w);
    c->updateShadow();
    w->updateShadow(c->shadow());
    connect(c, &Toplevel::shadowChanged, this,
        [w] {
            w->invalidateQuadsCache();
        }
    );
}

void Scene::removeToplevel(Toplevel *toplevel)
{
    Q_ASSERT(m_windows.contains(toplevel));
    delete m_windows.take(toplevel);
    toplevel->effectWindow()->setSceneWindow(nullptr);
}

void Scene::windowClosed(Toplevel *toplevel, Deleted *deleted)
{
    if (!deleted) {
        removeToplevel(toplevel);
        return;
    }

    Q_ASSERT(m_windows.contains(toplevel));
    Window *window = m_windows.take(toplevel);
    window->setToplevel(deleted);
    if (window->shadow()) {
        window->shadow()->setToplevel(deleted);
    }
    m_windows[deleted] = window;
}

void Scene::windowGeometryShapeChanged(Toplevel *c)
{
    if (!m_windows.contains(c))    // this is ok, shape is not valid by default
        return;
    Window *w = m_windows[ c ];
    w->discardShape();
}

void Scene::createStackingOrder(ToplevelList toplevels)
{
    // TODO: cache the stacking_order in case it has not changed
    foreach (Toplevel *c, toplevels) {
        Q_ASSERT(m_windows.contains(c));
        stacking_order.append(m_windows[ c ]);
    }
}

void Scene::clearStackingOrder()
{
    stacking_order.clear();
}

static Scene::Window *s_recursionCheck = nullptr;

void Scene::paintWindow(Window* w, int mask, QRegion region, WindowQuadList quads)
{
    // no painting outside visible screen (and no transformations)
    const QSize &screenSize = screens()->size();
    region &= QRect(0, 0, screenSize.width(), screenSize.height());
    if (region.isEmpty())  // completely clipped
        return;
    if (w->toplevel()->isDeleted() && w->toplevel()->skipsCloseAnimation()) {
        // should not get painted
        return;
    }

    if (s_recursionCheck == w) {
        return;
    }

    WindowPaintData data(w->toplevel()->effectWindow(), screenProjectionMatrix());
    data.quads = quads;
    effects->paintWindow(effectWindow(w), mask, region, data);
    // paint thumbnails on top of window
    paintWindowThumbnails(w, region, data.opacity(), data.brightness(), data.saturation());
    // and desktop thumbnails
    paintDesktopThumbnails(w);
}

static void adjustClipRegion(AbstractThumbnailItem *item, QRegion &clippingRegion)
{
    if (item->clip() && item->clipTo()) {
        // the x/y positions of the parent item are not correct. The margins are added, though the size seems fine
        // that's why we have to get the offset by inspecting the anchors properties
        QQuickItem *parentItem = item->clipTo();
        QPointF offset;
        QVariant anchors = parentItem->property("anchors");
        if (anchors.isValid()) {
            if (QObject *anchorsObject = anchors.value<QObject*>()) {
                offset.setX(anchorsObject->property("leftMargin").toReal());
                offset.setY(anchorsObject->property("topMargin").toReal());
            }
        }
        QRectF rect = QRectF(parentItem->position() - offset, QSizeF(parentItem->width(), parentItem->height()));
        if (QQuickItem *p = parentItem->parentItem()) {
            rect = p->mapRectToScene(rect);
        }
        clippingRegion &= rect.adjusted(0,0,-1,-1).translated(item->window()->position()).toRect();
    }
}

void Scene::paintWindowThumbnails(Scene::Window *w, QRegion region, qreal opacity, qreal brightness, qreal saturation)
{
    EffectWindowImpl *wImpl = static_cast<EffectWindowImpl*>(effectWindow(w));
    for (QHash<WindowThumbnailItem*, QPointer<EffectWindowImpl> >::const_iterator it = wImpl->thumbnails().constBegin();
            it != wImpl->thumbnails().constEnd();
            ++it) {
        if (it.value().isNull()) {
            continue;
        }
        WindowThumbnailItem *item = it.key();
        if (!item->isVisible()) {
            continue;
        }
        EffectWindowImpl *thumb = it.value().data();
        WindowPaintData thumbData(thumb, screenProjectionMatrix());
        thumbData.setOpacity(opacity);
        thumbData.setBrightness(brightness * item->brightness());
        thumbData.setSaturation(saturation * item->saturation());

        const QRect visualThumbRect(thumb->expandedGeometry());

        QSizeF size = QSizeF(visualThumbRect.size());
        size.scale(QSizeF(item->width(), item->height()), Qt::KeepAspectRatio);
        if (size.width() > visualThumbRect.width() || size.height() > visualThumbRect.height()) {
            size = QSizeF(visualThumbRect.size());
        }
        thumbData.setXScale(size.width() / static_cast<qreal>(visualThumbRect.width()));
        thumbData.setYScale(size.height() / static_cast<qreal>(visualThumbRect.height()));

        if (!item->window()) {
            continue;
        }
        const QPointF point = item->mapToScene(item->position());
        qreal x = point.x() + w->x() + (item->width() - size.width())/2;
        qreal y = point.y() + w->y() + (item->height() - size.height()) / 2;
        x -= thumb->x();
        y -= thumb->y();
        // compensate shadow topleft padding
        x += (thumb->x()-visualThumbRect.x())*thumbData.xScale();
        y += (thumb->y()-visualThumbRect.y())*thumbData.yScale();
        thumbData.setXTranslation(x);
        thumbData.setYTranslation(y);
        int thumbMask = PAINT_WINDOW_TRANSFORMED | PAINT_WINDOW_LANCZOS;
        if (thumbData.opacity() == 1.0) {
            thumbMask |= PAINT_WINDOW_OPAQUE;
        } else {
            thumbMask |= PAINT_WINDOW_TRANSLUCENT;
        }
        QRegion clippingRegion = region;
        clippingRegion &= QRegion(wImpl->x(), wImpl->y(), wImpl->width(), wImpl->height());
        adjustClipRegion(item, clippingRegion);
        effects->drawWindow(thumb, thumbMask, clippingRegion, thumbData);
    }
}

void Scene::paintDesktopThumbnails(Scene::Window *w)
{
    EffectWindowImpl *wImpl = static_cast<EffectWindowImpl*>(effectWindow(w));
    for (QList<DesktopThumbnailItem*>::const_iterator it = wImpl->desktopThumbnails().constBegin();
            it != wImpl->desktopThumbnails().constEnd();
            ++it) {
        DesktopThumbnailItem *item = *it;
        if (!item->isVisible()) {
            continue;
        }
        if (!item->window()) {
            continue;
        }
        s_recursionCheck = w;

        ScreenPaintData data;
        const QSize &screenSize = screens()->size();
        QSize size = screenSize;

        size.scale(item->width(), item->height(), Qt::KeepAspectRatio);
        data *= QVector2D(size.width() / double(screenSize.width()),
                          size.height() / double(screenSize.height()));
        const QPointF point = item->mapToScene(item->position());
        const qreal x = point.x() + w->x() + (item->width() - size.width())/2;
        const qreal y = point.y() + w->y() + (item->height() - size.height()) / 2;
        const QRect region = QRect(x, y, item->width(), item->height());
        QRegion clippingRegion = region;
        clippingRegion &= QRegion(wImpl->x(), wImpl->y(), wImpl->width(), wImpl->height());
        adjustClipRegion(item, clippingRegion);
        data += QPointF(x, y);
        const int desktopMask = PAINT_SCREEN_TRANSFORMED | PAINT_WINDOW_TRANSFORMED | PAINT_SCREEN_BACKGROUND_FIRST;
        paintDesktop(item->desktop(), desktopMask, clippingRegion, data);
        s_recursionCheck = nullptr;
    }
}

void Scene::paintDesktop(int desktop, int mask, const QRegion &region, ScreenPaintData &data)
{
    static_cast<EffectsHandlerImpl*>(effects)->paintDesktop(desktop, mask, region, data);
}

// the function that'll be eventually called by paintWindow() above
void Scene::finalPaintWindow(EffectWindowImpl* w, int mask, QRegion region, WindowPaintData& data)
{
    effects->drawWindow(w, mask, region, data);
}

// will be eventually called from drawWindow()
void Scene::finalDrawWindow(EffectWindowImpl* w, int mask, QRegion region, WindowPaintData& data)
{
    if (waylandServer() && waylandServer()->isScreenLocked() && !w->window()->isLockScreen() && !w->window()->isInputMethod()) {
        return;
    }
    w->sceneWindow()->performPaint(mask, region, data);
}

void Scene::extendPaintRegion(QRegion &region, bool opaqueFullscreen)
{
    Q_UNUSED(region);
    Q_UNUSED(opaqueFullscreen);
}

void Scene::screenGeometryChanged(const QSize &size)
{
    if (!overlayWindow()) {
        return;
    }
    overlayWindow()->resize(size);
}

bool Scene::makeOpenGLContextCurrent()
{
    return false;
}

void Scene::doneOpenGLContextCurrent()
{
}

void Scene::triggerFence()
{
}

QMatrix4x4 Scene::screenProjectionMatrix() const
{
    return QMatrix4x4();
}

xcb_render_picture_t Scene::xrenderBufferPicture() const
{
    return XCB_RENDER_PICTURE_NONE;
}

QPainter *Scene::scenePainter() const
{
    return nullptr;
}

QImage *Scene::qpainterRenderBuffer() const
{
    return nullptr;
}

QVector<QByteArray> Scene::openGLPlatformInterfaceExtensions() const
{
    return QVector<QByteArray>{};
}

//****************************************
// Scene::Window
//****************************************

Scene::Window::Window(Toplevel *toplevel)
    : m_toplevel(toplevel)
{
}

Scene::Window::~Window()
{
    delete m_shadow;
}

int Scene::Window::x() const
{
    return m_toplevel->x();
}

int Scene::Window::y() const
{
    return m_toplevel->y();
}

int Scene::Window::width() const
{
    return m_toplevel->width();
}

int Scene::Window::height() const
{
    return m_toplevel->height();
}

QRect Scene::Window::geometry() const
{
    return m_toplevel->frameGeometry();
}

QSize Scene::Window::size() const
{
    return m_toplevel->size();
}

QPoint Scene::Window::pos() const
{
    return m_toplevel->pos();
}

QRect Scene::Window::rect() const
{
    return m_toplevel->rect();
}

Toplevel *Scene::Window::toplevel() const
{
    return m_toplevel;
}

void Scene::Window::setToplevel(Toplevel *toplevel)
{
    m_toplevel = toplevel;
}

void Scene::Window::discardShape()
{
    // it is created on-demand and cached, simply reset the flag
    m_shapeIsValid = false;
    invalidateQuadsCache();
}

// Find out the shape of the window using the XShape extension
// or if shape is not set then simply it's the window geometry.
const QRegion &Scene::Window::shape() const
{
    if (!m_shapeIsValid) {
        if (toplevel()->shape()) {
            auto cookie = xcb_shape_get_rectangles_unchecked(connection(), toplevel()->frameId(), XCB_SHAPE_SK_BOUNDING);
            ScopedCPointer<xcb_shape_get_rectangles_reply_t> reply(xcb_shape_get_rectangles_reply(connection(), cookie, nullptr));
            if (!reply.isNull()) {
                m_shapeRegion = QRegion();
                auto *rects = xcb_shape_get_rectangles_rectangles(reply.data());
                for (int i = 0;
                        i < xcb_shape_get_rectangles_rectangles_length(reply.data());
                        ++i)
                    m_shapeRegion += QRegion(rects[i].x, rects[i].y,
                                             rects[i].width, rects[i].height);
                // make sure the shape is sane (X is async, maybe even XShape is broken)
                m_shapeRegion &= QRegion(0, 0, width(), height());
            } else {
                m_shapeRegion = QRegion();
            }
        } else {
            m_shapeRegion = QRegion(0, 0, width(), height());
        }
        m_shapeIsValid = true;
    }
    return m_shapeRegion;
}

QRegion Scene::Window::clientShape() const
{
    if (AbstractClient *client = qobject_cast<AbstractClient *>(m_toplevel)) {
        if (client->isShade()) {
            return QRegion();
        }
    }

    // TODO: cache
    const QRegion r = shape() & QRect(m_toplevel->clientPos(), m_toplevel->clientSize());
    return r.isEmpty() ? QRegion() : r;
}

bool Scene::Window::isVisible() const
{
    if (m_toplevel->isDeleted()) {
        return false;
    }
    if (!m_toplevel->isOnCurrentDesktop()) {
        return false;
    }
    if (!m_toplevel->isOnCurrentActivity()) {
        return false;
    }
    if (AbstractClient *client = qobject_cast<AbstractClient *>(m_toplevel)) {
        return client->isShown(true);
    }
    return true; // Unmanaged is always visible
}

bool Scene::Window::isOpaque() const
{
    return m_toplevel->opacity() == 1.0 && !m_toplevel->hasAlpha();
}

bool Scene::Window::isPaintingEnabled() const
{
    return !m_disablePainting;
}

void Scene::Window::resetPaintingEnabled()
{
    m_disablePainting = 0;
    if (m_toplevel->isDeleted()) {
        m_disablePainting |= PAINT_DISABLED_BY_DELETE;
    }
    if (static_cast<EffectsHandlerImpl *>(effects)->isDesktopRendering()) {
        if (!m_toplevel->isOnDesktop(static_cast<EffectsHandlerImpl *>(effects)->currentRenderedDesktop())) {
            m_disablePainting |= PAINT_DISABLED_BY_DESKTOP;
        }
    } else {
        if (!m_toplevel->isOnCurrentDesktop()) {
            m_disablePainting |= PAINT_DISABLED_BY_DESKTOP;
        }
    }
    if (!m_toplevel->isOnCurrentActivity()) {
        m_disablePainting |= PAINT_DISABLED_BY_ACTIVITY;
    }
    if (AbstractClient *client = qobject_cast<AbstractClient *>(m_toplevel)) {
        if (client->isMinimized()) {
            m_disablePainting |= PAINT_DISABLED_BY_MINIMIZE;
        }
        if (client->isHiddenInternal()) {
            m_disablePainting |= PAINT_DISABLED;
        }
    }
}

void Scene::Window::enablePainting(int reason)
{
    m_disablePainting &= ~reason;
}

void Scene::Window::disablePainting(int reason)
{
    m_disablePainting |= reason;
}

WindowQuadList Scene::Window::buildQuads(bool force) const
{
    if (m_cachedWindowQuads && !force) {
        return *m_cachedWindowQuads;
    }

    WindowQuadList ret;

    if (m_shadow && m_toplevel->wantsShadowToBeRendered()) {
        ret << m_shadow->windowQuads();
    }

    effects->buildQuads(m_toplevel->effectWindow(), ret);
    m_cachedWindowQuads.reset(new WindowQuadList(ret));

    return ret;
}

void Scene::Window::invalidateQuadsCache()
{
    m_cachedWindowQuads.reset();
}

ShadowSceneNode *Scene::Window::shadowNode() const
{
    return m_shadowNode;
}

DecorationSceneNode *Scene::Window::decorationNode() const
{
    return m_decorationNode;
}

SurfaceSceneNode *Scene::Window::surfaceNode() const
{
    return m_surfaceNode;
}

QVector<SceneNode *> Scene::Window::paintOrderNodes() const
{
    return m_paintOrderNodes;
}

void Scene::Window::updateShadow(Shadow* shadow)
{
    if (m_shadow == shadow) {
        return;
    }
    delete m_shadow;
    m_shadow = shadow;
}

const Shadow *Scene::Window::shadow() const
{
    return m_shadow;
}

Shadow *Scene::Window::shadow()
{
    return m_shadow;
}

//****************************************
// Scene::EffectFrame
//****************************************
Scene::EffectFrame::EffectFrame(EffectFrameImpl* frame)
    : m_effectFrame(frame)
{
}

Scene::EffectFrame::~EffectFrame()
{
}

SceneFactory::SceneFactory(QObject *parent)
    : QObject(parent)
{
}

SceneFactory::~SceneFactory()
{
}

SceneNode::SceneNode()
{
}

SceneNode::~SceneNode()
{
}

SceneNode *SceneNode::parent() const
{
    return m_parent;
}

SceneNode *SceneNode::firstChild() const
{
    return m_firstChild;
}

SceneNode *SceneNode::lastChild() const
{
    return m_lastChild;
}

void SceneNode::prependChildNode(SceneNode *node)
{
    if (m_firstChild) {
        m_firstChild->m_previousSibling = node;
    } else {
        m_lastChild = node;
    }

    node->m_nextSibling = m_firstChild;
    node->m_parent = this;

    m_firstChild = node;
}

void SceneNode::appendChildNode(SceneNode *node)
{
    if (m_lastChild) {
        m_lastChild->m_nextSibling = node;
    } else {
        m_firstChild = node;
    }

    node->m_previousSibling = m_lastChild;
    node->m_parent = this;

    m_lastChild = node;
}

void SceneNode::removeChildNode(SceneNode *node)
{
    SceneNode *previousNode = node->m_previousSibling;
    SceneNode *nextNode = node->m_nextSibling;

    if (previousNode) {
        previousNode->m_nextSibling = nextNode;
    } else {
        m_firstChild = nextNode;
    }

    if (nextNode) {
        nextNode->m_previousSibling = previousNode;
    } else {
        m_lastChild = previousNode;
    }

    node->m_previousSibling = nullptr;
    node->m_nextSibling = nullptr;
    node->m_parent = nullptr;
}

void SceneNode::insertChildNodeAfter(SceneNode *node, SceneNode *after)
{
    SceneNode *nextNode = after->m_nextSibling;
    if (nextNode) {
        nextNode->m_previousSibling = node;
    } else {
        m_lastChild = node;
    }

    node->m_previousSibling = after;
    node->m_nextSibling = nextNode;
    node->m_parent = this;

    after->m_nextSibling = node;
}

void SceneNode::insertChildNodeBefore(SceneNode *node, SceneNode *before)
{
    SceneNode *previousNode = before->m_previousSibling;
    if (previousNode) {
        previousNode->m_nextSibling = node;
    } else {
        m_firstChild = node;
    }

    node->m_previousSibling = previousNode;
    node->m_nextSibling = before;
    node->m_parent = this;

    before->m_previousSibling = node;
}

Toplevel *SceneNode::toplevel() const
{
    return m_toplevel;
}

void SceneNode::setToplevel(Toplevel *toplevel)
{
    m_toplevel = toplevel;
}

WindowQuadList SceneNode::windowQuads() const
{
    return m_windowQuads;
}

void SceneNode::setWindowQuads(const WindowQuadList &windowQuads)
{
    m_windowQuads = windowQuads;
}

QPoint SceneNode::position() const
{
    return m_position;
}

void SceneNode::setPosition(const QPoint &position)
{
    m_position = position;
}

QPoint SceneNode::origin() const
{
    return m_origin;
}

void SceneNode::setOrigin(const QPoint &origin)
{
    m_origin = origin;
}

ShadowSceneNode::~ShadowSceneNode()
{
}

Shadow *ShadowSceneNode::shadow() const
{
    return m_shadow;
}

void ShadowSceneNode::updateQuads()
{
    setWindowQuads(m_shadow->windowQuads());
}

SurfaceSceneNode::~SurfaceSceneNode()
{
}

PlatformSurface *SurfaceSceneNode::platformSurface() const
{
    return m_platformSurface;
}

PlatformSurface::PlatformSurface(SurfaceSceneNode *surfaceNode)
    : m_surfaceNode(surfaceNode)
{
}

PlatformSurface::~PlatformSurface()
{
}

Toplevel *PlatformSurface::toplevel() const
{
    return m_surfaceNode->toplevel();
}

X11PlatformSurface::X11PlatformSurface(SurfaceSceneNode *surfaceNode)
    : PlatformSurface(surfaceNode)
{
}

X11PlatformSurface::~X11PlatformSurface()
{
    if (m_pixmap != XCB_PIXMAP_NONE) {
        xcb_free_pixmap(connection(), m_pixmap);
    }
}

xcb_pixmap_t X11PlatformSurface::pixmap() const
{
    return m_pixmap;
}

QSize X11PlatformSurface::size() const
{
    return m_size;
}

WaylandPlatformSurface::WaylandPlatformSurface(SurfaceSceneNode *surfaceNode)
    : PlatformSurface(surfaceNode)
{
}

WaylandPlatformSurface::~WaylandPlatformSurface()
{
}

KWayland::Server::BufferInterface *WaylandPlatformSurface::buffer() const
{
    return m_buffer;
}

KWayland::Server::SurfaceInterface *WaylandPlatformSurface::surface() const
{
    return m_surface;
}

InternalPlatformSurface::InternalPlatformSurface(SurfaceSceneNode *surfaceNode)
    : PlatformSurface(surfaceNode)
{
}

InternalPlatformSurface::~InternalPlatformSurface()
{
}

QSharedPointer<QOpenGLFramebufferObject> InternalPlatformSurface::framebufferObject() const
{
    return m_framebufferObject;
}

QImage InternalPlatformSurface::image() const
{
    return m_image;
}

} // namespace
