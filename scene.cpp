/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>

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
    if (mask & (PAINT_SCREEN_TRANSFORMED | PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS) || true)
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
        Toplevel* topw = w->window();

        // Reset the repaint_region.
        // This has to be done here because many effects schedule a repaint for
        // the next frame within Effects::prePaintWindow.
        topw->resetRepaints();

        WindowPrePaintData data;
        data.mask = orig_mask | (w->isOpaque() ? PAINT_WINDOW_OPAQUE : PAINT_WINDOW_TRANSLUCENT);
        w->resetPaintingEnabled();
        data.paint = infiniteRegion(); // no clipping, so doesn't really matter
        data.clip = QRegion();
        data.quads = w->windowQuads();
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

#if 0
    QRegion dirtyArea = region;
    bool opaqueFullscreen = false;

    // Traverse the scene windows from bottom to top.
    for (int i = 0; i < stacking_order.count(); ++i) {
        Window *window = stacking_order[i];
        Toplevel *toplevel = window->window();
        WindowPrePaintData data;
        data.mask = orig_mask | (window->isOpaque() ? PAINT_WINDOW_OPAQUE : PAINT_WINDOW_TRANSLUCENT);
        window->resetPaintingEnabled();
        data.paint = region;
        data.paint |= toplevel->repaints();

        // Reset the repaint_region.
        // This has to be done here because many effects schedule a repaint for
        // the next frame within Effects::prePaintWindow.
        toplevel->resetRepaints();

        // Clip out the decoration for opaque windows; the decoration is drawn in the second pass
        opaqueFullscreen = false; // TODO: do we care about unmanged windows here (maybe input windows?)
        if (window->isOpaque()) {
            AbstractClient *client = dynamic_cast<AbstractClient *>(toplevel);
            if (client) {
                opaqueFullscreen = client->isFullScreen();
            }
            if (!(client && client->decorationHasAlpha())) {
                data.clip = window->decorationShape().translated(window->pos());
            }
            data.clip |= window->clientShape().translated(window->pos() + window->bufferOffset());
        } else if (toplevel->hasAlpha() && toplevel->opacity() == 1.0) {
            const QRegion clientShape = window->clientShape().translated(window->pos() + window->bufferOffset());
            const QRegion opaqueShape = toplevel->opaqueRegion().translated(window->pos() + toplevel->clientPos());
            data.clip = clientShape & opaqueShape;
        } else {
            data.clip = QRegion();
        }
        data.quads = w->windowQuads();
        // preparation step
        effects->prePaintWindow(effectWindow(window), data, time_diff);
#if !defined(QT_NO_DEBUG)
        if (data.quads.isTransformed()) {
            qFatal("Pre-paint calls are not allowed to transform quads!");
        }
#endif
        if (!window->isPaintingEnabled()) {
            continue;
        }
        dirtyArea |= data.paint;
        // Schedule the window for painting
        phase2data.append({ window, data.paint, data.clip, data.mask, data.quads });
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

        if (fullRepaint) {
            data->region = displayRegion;
        } else {
            data->region |= upperTranslucentDamage;
        }

        // subtract the parts which will possibly been drawn as part of
        // a higher opaque window
        data->region -= allclips;

        // Here we rely on WindowPrePaintData::setTranslucent() to remove
        // the clip if needed.
        if (!data->clip.isEmpty() && !(data->mask & PAINT_WINDOW_TRANSLUCENT)) {
            // clip away the opaque regions for all windows below this one
            allclips |= data->clip;
            // extend the translucent damage for windows below this by remaining (translucent) regions
            if (!fullRepaint) {
                upperTranslucentDamage |= data->region - data->clip;
            }
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
#endif
}

void Scene::addToplevel(Toplevel *toplevel)
{
    Q_ASSERT(!m_windows.contains(toplevel));

    using namespace KWayland::Server;

    Scene::Window *window = createWindow(toplevel);
    m_windows[toplevel] = window;

    toplevel->effectWindow()->setSceneWindow(window);
    toplevel->updateShadow();
    window->updateShadow(toplevel->shadow());

    connect(toplevel, &Toplevel::windowClosed, this, &Scene::replaceToplevel);

    // FIXME: Perhaps move scene node tracking into a dedicated class.

    if (SurfaceInterface *surface = toplevel->surface()) {
        connect(surface, &SurfaceInterface::subSurfaceTreeChanged, this, [window] {
           window->markDirtyAttributes(Window::Children);
        });
    }

    connect(toplevel, &Toplevel::geometryShapeChanged, this, [window] {
        window->markDirtyAttributes(Window::Geometry | Window::Shape);
    });
    connect(toplevel, &Toplevel::screenScaleChanged, this, [window] {
        window->markDirtyAttributes(Window::Quads);
    });
    connect(toplevel, &Toplevel::shadowChanged, this, [window] {
         window->markDirtyAttributes(Window::Children);
    });
}

void Scene::removeToplevel(Toplevel *toplevel)
{
    Q_ASSERT(m_windows.contains(toplevel));
    delete m_windows.take(toplevel);
    toplevel->effectWindow()->setSceneWindow(nullptr);
}

void Scene::replaceToplevel(Toplevel *toplevel, Deleted *deleted)
{
    if (!deleted) {
        removeToplevel(toplevel);
        return;
    }

    Q_ASSERT(m_windows.contains(toplevel));
    Window *window = m_windows.take(toplevel);
    window->updateToplevel(deleted);
    if (window->shadow()) {
        window->shadow()->setToplevel(deleted);
    }
    m_windows[deleted] = window;
}

void Scene::createStackingOrder(QList<Toplevel *> toplevels)
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
    if (w->window()->isDeleted() && w->window()->skipsCloseAnimation()) {
        // should not get painted
        return;
    }

    if (s_recursionCheck == w) {
        return;
    }

    WindowPaintData data(w->window()->effectWindow(), screenProjectionMatrix());
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

Scene::Window::Window(Toplevel * c)
    : toplevel(c)
    , m_nodeUpdater(new SceneNodeUpdater(this))
{
}

Scene::Window::~Window()
{
    delete m_nodeUpdater;
    delete m_rootNode;
    delete m_shadow; // FIXME: The shadow node most likely needs to take ownership of the shadow.
}

bool Scene::Window::isVisible() const
{
    if (toplevel->isDeleted())
        return false;
    if (!toplevel->isOnCurrentDesktop())
        return false;
    if (!toplevel->isOnCurrentActivity())
        return false;
    if (AbstractClient *c = dynamic_cast<AbstractClient*>(toplevel))
        return c->isShown(true);
    return true; // Unmanaged is always visible
}

bool Scene::Window::isOpaque() const
{
    return toplevel->opacity() == 1.0 && !toplevel->hasAlpha();
}

bool Scene::Window::isPaintingEnabled() const
{
    return !m_disablePainting;
}

void Scene::Window::resetPaintingEnabled()
{
    m_disablePainting = 0;
    if (toplevel->isDeleted()) {
        m_disablePainting |= PAINT_DISABLED_BY_DELETE;
    }
    if (static_cast<EffectsHandlerImpl *>(effects)->isDesktopRendering()) {
        if (!toplevel->isOnDesktop(static_cast<EffectsHandlerImpl*>(effects)->currentRenderedDesktop())) {
            m_disablePainting |= PAINT_DISABLED_BY_DESKTOP;
        }
    } else {
        if (!toplevel->isOnCurrentDesktop()) {
            m_disablePainting |= PAINT_DISABLED_BY_DESKTOP;
        }
    }
    if (!toplevel->isOnCurrentActivity()) {
        m_disablePainting |= PAINT_DISABLED_BY_ACTIVITY;
    }
    if (AbstractClient *client = qobject_cast<AbstractClient *>(toplevel)) {
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

RootSceneNode *Scene::Window::rootNode() const
{
    return m_rootNode;
}

WindowQuadList Scene::Window::windowQuads() const
{
    return m_windowQuads;
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

void Scene::Window::markDirtyAttributes(int attributes)
{
    m_dirtyAttributes |= attributes;
}

void Scene::Window::updateDirtyAttributes()
{
    const int dirtyAttributes = m_dirtyAttributes;
    m_dirtyAttributes = 0;

    if (toplevel->isDeleted()) {
        return;
    }

    if (!m_rootNode) {
        m_rootNode = new RootSceneNode(toplevel);
        m_rootNode->setVisible(true);
    }

    if (dirtyAttributes & Children) {
        m_rootNode->updateChildren();
        m_paintOrderNodes = m_rootNode->subtreeNodes();
    }

    if (dirtyAttributes & GenericUpdateMask) {
        m_nodeUpdater->update(dirtyAttributes);
    }

    if (dirtyAttributes & QuadsUpdateMask) {
        // TODO: Update window quads.
    }
}

//****************************************
// WindowPixmap
//****************************************
WindowPixmap::WindowPixmap(Scene::Window *window)
    : m_window(window)
{
}

WindowPixmap::~WindowPixmap()
{
    if (m_pixmap != XCB_WINDOW_NONE) {
        xcb_free_pixmap(connection(), m_pixmap);
    }
    if (m_buffer) {
        using namespace KWayland::Server;
        QObject::disconnect(m_buffer.data(), &BufferInterface::aboutToBeDestroyed, m_buffer.data(), &BufferInterface::unref);
        m_buffer->unref();
    }
}

void WindowPixmap::create()
{
    if (isValid() || toplevel()->isDeleted()) {
        return;
    }
    // always update from Buffer on Wayland, don't try using XPixmap
    if (kwinApp()->shouldUseWaylandForCompositing()) {
        // use Buffer
        updateBuffer();
        return;
    }
    XServerGrabber grabber;
    xcb_pixmap_t pixmap = xcb_generate_id(connection());
    xcb_void_cookie_t namePixmapCookie = xcb_composite_name_window_pixmap_checked(connection(), toplevel()->windowId(), pixmap);
    Xcb::WindowAttributes windowAttributes(toplevel()->windowId());
    Xcb::WindowGeometry windowGeometry(toplevel()->windowId());
    if (xcb_generic_error_t *error = xcb_request_check(connection(), namePixmapCookie)) {
        qCDebug(KWIN_CORE) << "Creating window pixmap failed: " << error->error_code;
        free(error);
        return;
    }
    // check that the received pixmap is valid and actually matches what we
    // know about the window (i.e. size)
    if (!windowAttributes || windowAttributes->map_state != XCB_MAP_STATE_VIEWABLE) {
        qCDebug(KWIN_CORE) << "Creating window pixmap failed: " << this;
        xcb_free_pixmap(connection(), pixmap);
        return;
    }
    const QRect bufferGeometry = toplevel()->bufferGeometry();
    if (windowGeometry.size() != bufferGeometry.size()) {
        qCDebug(KWIN_CORE) << "Creating window pixmap failed: " << this;
        xcb_free_pixmap(connection(), pixmap);
        return;
    }
    m_pixmap = pixmap;
}

bool WindowPixmap::isValid() const
{
    if (!m_buffer.isNull() || !m_fbo.isNull() || !m_internalImage.isNull()) {
        return true;
    }
    return m_pixmap != XCB_PIXMAP_NONE;
}

void WindowPixmap::updateBuffer()
{
    using namespace KWayland::Server;
    if (surface()) {
        if (BufferInterface *buffer = surface()->buffer()) {
            if (m_buffer == buffer) {
                // no change
                return;
            }
            if (m_buffer) {
                QObject::disconnect(m_buffer.data(), &BufferInterface::aboutToBeDestroyed, m_buffer.data(), &BufferInterface::unref);
                m_buffer->unref();
            }
            m_buffer = buffer;
            m_buffer->ref();
            QObject::connect(m_buffer.data(), &BufferInterface::aboutToBeDestroyed, m_buffer.data(), &BufferInterface::unref);
        }
    } else if (toplevel()->internalFramebufferObject()) {
        m_fbo = toplevel()->internalFramebufferObject();
    } else if (!toplevel()->internalImageObject().isNull()) {
        m_internalImage = toplevel()->internalImageObject();
    } else {
        if (m_buffer) {
            QObject::disconnect(m_buffer.data(), &BufferInterface::aboutToBeDestroyed, m_buffer.data(), &BufferInterface::unref);
            m_buffer->unref();
            m_buffer.clear();
        }
    }
}

KWayland::Server::SurfaceInterface *WindowPixmap::surface() const
{
    return m_surface;
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

//****************************************
// SceneNode
//****************************************
SceneNode::SceneNode(Toplevel *toplevel, NodeType nodeType)
    : m_toplevel(toplevel)
    , m_nodeType(nodeType)
{
}

SceneNode::~SceneNode()
{
    if (m_parent) {
        m_parent->removeChildNode(this);
    }
    while (m_firstChild) {
        delete m_firstChild;
    }
}

SceneNode::NodeType SceneNode::type() const
{
    return m_nodeType;
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

SceneNode *SceneNode::previousSibling() const
{
    return m_previousSibling;
}

SceneNode *SceneNode::nextSibling() const
{
    return m_nextSibling;
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

QVector<SceneNode *> SceneNode::subtreeNodes() const
{
    QVector<SceneNode *> nodes;

    // FIXME: Use a more efficient algorithm?
    for (SceneNode *child = firstChild(); child; child = child->nextSibling()) {
        nodes << child << child->subtreeNodes();
    }

    return nodes;
}

Scene::Window *SceneNode::sceneWindow() const
{
    return toplevel()->effectWindow()->sceneWindow();
}

Toplevel *SceneNode::toplevel() const
{
    return m_toplevel;
}

void SceneNode::setToplevel(Toplevel *toplevel)
{
    forEachChild([=](SceneNode *child) { child->setToplevel(toplevel); });
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

QPoint SceneNode::combinedPosition() const
{
    return m_combinedPosition;
}

void SceneNode::setCombinedPosition(const QPoint &position)
{
    m_combinedPosition = position;
}

QSize SceneNode::size() const
{
    return m_size;
}

void SceneNode::setSize(const QSize &size)
{
    m_size = size;
}

QRect SceneNode::rect() const
{
    return QRect(QPoint(), m_size);
}

QRegion SceneNode::shape() const
{
    return m_shape;
}

void SceneNode::setShape(const QRegion &shape)
{
    m_shape = shape;
}

int SceneNode::id() const
{
    return m_id;
}

void SceneNode::setId(int id)
{
    m_id = id;
}

bool SceneNode::isVisible() const
{
    return m_isVisible;
}

void SceneNode::setVisible(bool visible)
{
    m_isVisible = visible;
}

void SceneNode::forEachChild(std::function<void (SceneNode *child)> func)
{
    for (SceneNode *child = firstChild(); child; child = child->nextSibling()) {
        func(child);
    }
}

void SceneNode::updateChildren()
{
}

void SceneNode::updateQuads()
{
}

void SceneNode::updateTexture()
{
}

ShadowSceneNode::ShadowSceneNode(Toplevel *toplevel)
    : SceneNode(toplevel, ShadowNodeType)
{
}

Shadow *ShadowSceneNode::shadow() const
{
    return m_shadow.data();
}

void ShadowSceneNode::setShadow(Shadow *shadow)
{
    m_shadow.reset(shadow);
}

void ShadowSceneNode::updateQuads()
{
    setWindowQuads(m_shadow->shadowQuads());
}

DecorationSceneNode::DecorationSceneNode(Toplevel *toplevel)
    : SceneNode(toplevel, DecorationNodeType)
{
}

void DecorationSceneNode::updateQuads()
{
}

SurfaceSceneNode::SurfaceSceneNode(Toplevel *toplevel)
    : SceneNode(toplevel, SurfaceNodeType)
{
}

KWayland::Server::SurfaceInterface *SurfaceSceneNode::surface() const
{
    return m_surface;
}

void SurfaceSceneNode::setSurface(KWayland::Server::SurfaceInterface *surface)
{
    m_surface = surface;
}

void SurfaceSceneNode::updateChildren()
{
    SceneNode *currentNode = firstChild();
    SceneNode *desiredNode = nullptr;

    int childIndex = 0;

    while ((desiredNode = fetchChildNode(childIndex++, currentNode))) {
        if (currentNode != desiredNode) {
            removeChildNode(desiredNode);
            insertChildNodeBefore(desiredNode, currentNode);
        } else {
            currentNode = desiredNode->nextSibling();
        }
        desiredNode->updateChildren();
    }

    while (currentNode) {
        SceneNode *nextNode = currentNode->nextSibling();
        delete currentNode;
        currentNode = nextNode;
    }
}

static KWayland::Server::SurfaceInterface *fetchChildSurface(KWayland::Server::SurfaceInterface *surface, int index)
{
    if (!surface) {
        return nullptr;
    }

    const KWayland::Server::SubSurfaceInterface *subSurface = surface->childSubSurfaces().value(index);
    if (!subSurface) {
        return nullptr;
    }

    return subSurface->surface();
}

static KWayland::Server::SurfaceInterface *surfaceFromNode(SceneNode *node)
{
    const SurfaceSceneNode *surfaceNode = dynamic_cast<SurfaceSceneNode *>(node);
    if (!surfaceNode) {
        return nullptr;
    }
    return surfaceNode->surface();
}

SceneNode *SurfaceSceneNode::fetchChildNode(int childIndex, SceneNode *currentNode) const
{
    KWayland::Server::SurfaceInterface *childSurface = fetchChildSurface(surface(), childIndex);
    if (!childSurface) {
        return nullptr;
    }

    while (currentNode) {
        if (surfaceFromNode(currentNode) == childSurface) {
            return currentNode;
        }
        currentNode = currentNode->nextSibling();
    }

    SurfaceSceneNode *surfaceNode = sceneWindow()->createSurfaceNode();
    surfaceNode->setSurface(childSurface);

    return surfaceNode;
}

void SurfaceSceneNode::updateQuads()
{
    const QRegion contentShape = shape();
    const QSizeF contentSize = size();

    WindowQuadList windowQuads;
    windowQuads.reserve(contentShape.rectCount());

    for (const QRectF &rect : contentShape) {
        const qreal x1 = rect.left();
        const qreal y1 = rect.top();
        const qreal x2 = rect.right();
        const qreal y2 = rect.bottom();

        const qreal u1 = x1 / contentSize.width();
        const qreal v1 = y1 / contentSize.height();
        const qreal u2 = x2 / contentSize.width();
        const qreal v2 = y2 / contentSize.height();

        WindowQuad windowQuad(WindowQuadContents, id());
        windowQuad[0] = WindowVertex(x1, y1, u1, v1);
        windowQuad[1] = WindowVertex(x2, y1, u2, v1);
        windowQuad[2] = WindowVertex(x2, y2, u2, v2);
        windowQuad[3] = WindowVertex(x1, y2, u1, v2);

        windowQuads << windowQuad;
    }

    setWindowQuads(windowQuads);
}

RootSceneNode::RootSceneNode(Toplevel *toplevel)
    : SceneNode(toplevel, RootNodeType)
{
}

ShadowSceneNode *RootSceneNode::shadowNode() const
{
    return m_shadowNode;
}

DecorationSceneNode *RootSceneNode::decorationNode() const
{
    return m_decorationNode;
}

SurfaceSceneNode *RootSceneNode::surfaceNode() const
{
    return m_surfaceNode;
}

void RootSceneNode::updateChildren()
{
    if (!m_surfaceNode) {
        m_surfaceNode = sceneWindow()->createSurfaceNode();
        m_surfaceNode->setSurface(toplevel()->surface());
        appendChildNode(m_surfaceNode);
    }

    m_surfaceNode->updateChildren();

    if (toplevel()->shadow()) {
        if (!m_shadowNode) {
            m_shadowNode = sceneWindow()->createShadowNode();
            m_shadowNode->setShadow(toplevel()->shadow());
            prependChildNode(m_shadowNode);
        }
        m_shadowNode->updateChildren();
    } else {
        delete m_shadowNode;
        m_shadowNode = nullptr;
    }

    if (auto client = qobject_cast<AbstractClient *>(toplevel())) {
        if (client->decoration()) {
            if (!m_decorationNode) {
                m_decorationNode = sceneWindow()->createDecorationNode();
                insertChildNodeBefore(m_decorationNode, lastChild());
            }
            m_decorationNode->updateChildren();
        } else {
            delete m_decorationNode;
            m_decorationNode = nullptr;
        }
    }
}

SceneNodeUpdater::SceneNodeUpdater(Scene::Window *window)
    : m_window(window)
{
}

void SceneNodeUpdater::update(int dirtyAttributes)
{
    m_dirtyAttributes = dirtyAttributes;
    m_lastNodeId = 0;

    visitChildren(m_window->rootNode());
}

void SceneNodeUpdater::enterShadowNode(ShadowSceneNode *shadowNode)
{
    const Shadow *shadow = shadowNode->shadow();
    if (m_dirtyAttributes & Scene::Window::Geometry) {

    }
    if (m_dirtyAttributes & Scene::Window::Visibility) {
        shadowNode->setVisible(shadowNode->toplevel()->wantsShadowToBeRendered());
    }
    if (m_dirtyAttributes & Scene::Window::Shape) {
        shadowNode->setShape(shadow->shadowRegion());
    }
}

void SceneNodeUpdater::leaveShadowNode(ShadowSceneNode *shadowNode)
{
}

void SceneNodeUpdater::enterDecorationNode(DecorationSceneNode *decorationNode)
{
}

void SceneNodeUpdater::leaveDecorationNode(DecorationSceneNode *decorationNode)
{
}

static QPoint determineSurfaceNodePosition(const SurfaceSceneNode *surfaceNode)
{
    const KWayland::Server::SurfaceInterface *surface = surfaceNode->surface();
    const Toplevel *toplevel = surfaceNode->toplevel();

    if (!surface || toplevel->surface() == surface) {
        const QRect bufferGeometry = toplevel->bufferGeometry();
        const QRect frameGeometry = toplevel->frameGeometry();
        return bufferGeometry.topLeft() - frameGeometry.topLeft();
    }

    const KWayland::Server::SubSurfaceInterface *subSurface = surface->subSurface();
    if (!subSurface) {
        return QPoint();
    }

    return subSurface->position();
}

static QSize determineSurfaceNodeSize(const SurfaceSceneNode *surfaceNode)
{
    const KWayland::Server::SurfaceInterface *surface = surfaceNode->surface();
    if (surface) {
        return surface->size();
    }
    return surfaceNode->toplevel()->bufferGeometry().size();
}

static bool determineSurfaceNodeVisibility(const SurfaceSceneNode *surfaceNode)
{
    if (const auto *client = qobject_cast<AbstractClient *>(surfaceNode->toplevel())) {
        if (client->isShade()) {
            return false;
        }
    }
    if (surfaceNode->surface()) {
        return surfaceNode->surface()->isMapped();
    }
    return true;
}

static QRegion determineSurfaceNodeShape(const SurfaceSceneNode *surfaceNode)
{
    const Toplevel *toplevel = surfaceNode->toplevel();
    if (toplevel->protocol() == Protocol::X11) {
        if (toplevel->isShaped()) {
            return toplevel->shape();
        }
    }
    return surfaceNode->rect();
}

void SceneNodeUpdater::enterSurfaceNode(SurfaceSceneNode *surfaceNode)
{
    if (m_dirtyAttributes & Scene::Window::Geometry) {
        surfaceNode->setPosition(determineSurfaceNodePosition(surfaceNode));
        surfaceNode->setSize(determineSurfaceNodeSize(surfaceNode));
    }
    if (m_dirtyAttributes & Scene::Window::Visibility) {
        surfaceNode->setVisible(determineSurfaceNodeVisibility(surfaceNode));
    }
    if (m_dirtyAttributes & Scene::Window::Shape) {
        surfaceNode->setShape(determineSurfaceNodeShape(surfaceNode));
    }
}

void SceneNodeUpdater::leaveSurfaceNode(SurfaceSceneNode *surfaceNode)
{
}

void SceneNodeUpdater::visitNode(SceneNode *node)
{
    if (m_dirtyAttributes & Scene::Window::Children) {
        node->setId(m_lastNodeId++);
    }

    switch (node->type()) {
    case SceneNode::ShadowNodeType: {
        ShadowSceneNode *shadowNode = static_cast<ShadowSceneNode *>(node);
        enterShadowNode(shadowNode);
        visitChildren(shadowNode);
        leaveShadowNode(shadowNode);
        break; }
    case SceneNode::DecorationNodeType: {
        DecorationSceneNode *decorationNode = static_cast<DecorationSceneNode *>(node);
        enterDecorationNode(decorationNode);
        visitChildren(decorationNode);
        leaveDecorationNode(decorationNode);
        break; }
    case SceneNode::SurfaceNodeType: {
        SurfaceSceneNode *surfaceNode = static_cast<SurfaceSceneNode *>(node);
        enterSurfaceNode(surfaceNode);
        visitChildren(surfaceNode);
        leaveSurfaceNode(surfaceNode);
        break; }
    case SceneNode::RootNodeType:
        break;
    }
}

void SceneNodeUpdater::visitChildren(SceneNode *node)
{
    node->forEachChild([this](SceneNode *child) { visitNode(child); });
}

} // namespace
