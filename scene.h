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

#ifndef KWIN_SCENE_H
#define KWIN_SCENE_H

#include "toplevel.h"
#include "utils.h"
#include "kwineffects.h"

#include <QElapsedTimer>
#include <QMatrix4x4>

class QOpenGLFramebufferObject;

namespace KWayland
{
namespace Server
{
class BufferInterface;
class SubSurfaceInterface;
}
}

namespace KWin
{

namespace Decoration
{
class DecoratedClientImpl;
class Renderer;
}

class AbstractThumbnailItem;
class DecorationSceneNode;
class Deleted;
class EffectFrameImpl;
class EffectWindowImpl;
class OverlayWindow;
class Shadow;
class ShadowSceneNode;
class SurfaceSceneNode;

// The base class for compositing backends.
class KWIN_EXPORT Scene : public QObject
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = nullptr);
    ~Scene() override = 0;
    class EffectFrame;
    class Window;

    // Returns true if the ctor failed to properly initialize.
    virtual bool initFailed() const = 0;
    virtual CompositingType compositingType() const = 0;

    virtual bool hasPendingFlush() const { return false; }

    // Repaints the given screen areas, windows provides the stacking order.
    // The entry point for the main part of the painting pass.
    // returns the time since the last vblank signal - if there's one
    // ie. "what of this frame is lost to painting"
    virtual qint64 paint(QRegion damage, ToplevelList windows) = 0;

    /**
     * Adds the Toplevel to the Scene.
     *
     * If the toplevel gets deleted, then the scene will try automatically
     * to re-bind an underlying scene window to the corresponding Deleted.
     *
     * @param toplevel The window to be added.
     * @note You can add a toplevel to scene only once.
     */
    void addToplevel(Toplevel *toplevel);

    /**
     * Removes the Toplevel from the Scene.
     *
     * @param toplevel The window to be removed.
     * @note You can remove a toplevel from the scene only once.
     */
    void removeToplevel(Toplevel *toplevel);

    /**
     * @brief Creates the Scene backend of an EffectFrame.
     *
     * @param frame The EffectFrame this Scene::EffectFrame belongs to.
     */
    virtual Scene::EffectFrame *createEffectFrame(EffectFrameImpl *frame) = 0;
    /**
     * @brief Creates the Scene specific Shadow subclass.
     *
     * An implementing class has to create a proper instance. It is not allowed to
     * return @c null.
     *
     * @param toplevel The Toplevel for which the Shadow needs to be created.
     */
    virtual Shadow *createShadow(Toplevel *toplevel) = 0;
    /**
     * Method invoked when the screen geometry is changed.
     * Reimplementing classes should also invoke the parent method
     * as it takes care of resizing the overlay window.
     * @param size The new screen geometry size
     */
    virtual void screenGeometryChanged(const QSize &size);
    // Flags controlling how painting is done.
    enum {
        // Window (or at least part of it) will be painted opaque.
        PAINT_WINDOW_OPAQUE         = 1 << 0,
        // Window (or at least part of it) will be painted translucent.
        PAINT_WINDOW_TRANSLUCENT    = 1 << 1,
        // Window will be painted with transformed geometry.
        PAINT_WINDOW_TRANSFORMED    = 1 << 2,
        // Paint only a region of the screen (can be optimized, cannot
        // be used together with TRANSFORMED flags).
        PAINT_SCREEN_REGION         = 1 << 3,
        // Whole screen will be painted with transformed geometry.
        PAINT_SCREEN_TRANSFORMED    = 1 << 4,
        // At least one window will be painted with transformed geometry.
        PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS = 1 << 5,
        // Clear whole background as the very first step, without optimizing it
        PAINT_SCREEN_BACKGROUND_FIRST = 1 << 6,
        // PAINT_DECORATION_ONLY = 1 << 7 has been removed
        // Window will be painted with a lanczos filter.
        PAINT_WINDOW_LANCZOS = 1 << 8
        // PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_WITHOUT_FULL_REPAINTS = 1 << 9 has been removed
    };
    // types of filtering available
    enum ImageFilterType { ImageFilterFast, ImageFilterGood };
    // there's nothing to paint (adjust time_diff later)
    virtual void idle();
    virtual OverlayWindow* overlayWindow() const = 0;

    virtual bool makeOpenGLContextCurrent();
    virtual void doneOpenGLContextCurrent();

    virtual QMatrix4x4 screenProjectionMatrix() const;

    /**
     * Whether the Scene uses an X11 overlay window to perform compositing.
     */
    virtual bool usesOverlayWindow() const = 0;

    virtual void triggerFence();

    virtual Decoration::Renderer *createDecorationRenderer(Decoration::DecoratedClientImpl *) = 0;

    /**
     * Whether the Scene is able to drive animations.
     * This is used as a hint to the effects system which effects can be supported.
     * If the Scene performs software rendering it is supposed to return @c false,
     * if rendering is hardware accelerated it should return @c true.
     */
    virtual bool animationsSupported() const = 0;

    /**
     * The render buffer used by an XRender based compositor scene.
     * Default implementation returns XCB_RENDER_PICTURE_NONE
     */
    virtual xcb_render_picture_t xrenderBufferPicture() const;

    /**
     * The QPainter used by a QPainter based compositor scene.
     * Default implementation returns @c nullptr;
     */
    virtual QPainter *scenePainter() const;

    /**
     * The render buffer used by a QPainter based compositor.
     * Default implementation returns @c nullptr.
     */
    virtual QImage *qpainterRenderBuffer() const;

    /**
     * The backend specific extensions (e.g. EGL/GLX extensions).
     *
     * Not the OpenGL (ES) extension!
     *
     * Default implementation returns empty list
     */
    virtual QVector<QByteArray> openGLPlatformInterfaceExtensions() const;

    /**
     *
     */
    virtual ShadowSceneNode *createShadowSceneNode() = 0;

    /**
     *
     */
    virtual DecorationSceneNode *createDecorationSceneNode() = 0;

    /**
     *
     */
    virtual SurfaceSceneNode *createSurfaceSceneNode() = 0;

Q_SIGNALS:
    void frameRendered();
    void resetCompositing();

public Q_SLOTS:
    // shape/size of a window changed
    void windowGeometryShapeChanged(KWin::Toplevel* c);
    // a window has been closed
    void windowClosed(KWin::Toplevel* c, KWin::Deleted* deleted);
protected:
    virtual Window *createWindow(Toplevel *toplevel) = 0;
    void createStackingOrder(ToplevelList toplevels);
    void clearStackingOrder();
    // shared implementation, starts painting the screen
    void paintScreen(int *mask, const QRegion &damage, const QRegion &repaint,
                     QRegion *updateRegion, QRegion *validRegion, const QMatrix4x4 &projection = QMatrix4x4(), const QRect &outputGeometry = QRect());
    // Render cursor texture in case hardware cursor is disabled/non-applicable
    virtual void paintCursor() = 0;
    friend class EffectsHandlerImpl;
    // called after all effects had their paintScreen() called
    void finalPaintScreen(int mask, QRegion region, ScreenPaintData& data);
    // shared implementation of painting the screen in the generic
    // (unoptimized) way
    virtual void paintGenericScreen(int mask, ScreenPaintData data);
    // shared implementation of painting the screen in an optimized way
    virtual void paintSimpleScreen(int mask, QRegion region);
    // paint the background (not the desktop background - the whole background)
    virtual void paintBackground(QRegion region) = 0;
    // called after all effects had their paintWindow() called
    void finalPaintWindow(EffectWindowImpl* w, int mask, QRegion region, WindowPaintData& data);
    // shared implementation, starts painting the window
    virtual void paintWindow(Window* w, int mask, QRegion region, WindowQuadList quads);
    // called after all effects had their drawWindow() called
    virtual void finalDrawWindow(EffectWindowImpl* w, int mask, QRegion region, WindowPaintData& data);
    // let the scene decide whether it's better to paint more of the screen, eg. in order to allow a buffer swap
    // the default is NOOP
    virtual void extendPaintRegion(QRegion &region, bool opaqueFullscreen);
    virtual void paintDesktop(int desktop, int mask, const QRegion &region, ScreenPaintData &data);

    virtual void paintEffectQuickView(EffectQuickView *w) = 0;

    // compute time since the last repaint
    void updateTimeDiff();
    // saved data for 2nd pass of optimized screen painting
    struct Phase2Data {
        Window *window = nullptr;
        QRegion region;
        QRegion clip;
        int mask = 0;
        WindowQuadList quads;
    };
    // The region which actually has been painted by paintScreen() and should be
    // copied from the buffer to the screen. I.e. the region returned from Scene::paintScreen().
    // Since prePaintWindow() can extend areas to paint, these changes would have to propagate
    // up all the way from paintSimpleScreen() up to paintScreen(), so save them here rather
    // than propagate them up in arguments.
    QRegion painted_region;
    // Additional damage that needs to be repaired to bring a reused back buffer up to date
    QRegion repaint_region;
    // The dirty region before it was unioned with repaint_region
    QRegion damaged_region;
    // time since last repaint
    int time_diff;
    QElapsedTimer last_time;
private:
    void paintWindowThumbnails(Scene::Window *w, QRegion region, qreal opacity, qreal brightness, qreal saturation);
    void paintDesktopThumbnails(Scene::Window *w);
    QHash< Toplevel*, Window* > m_windows;
    // windows in their stacking order
    QVector< Window* > stacking_order;
};

/**
 * Factory class to create a Scene. Needs to be implemented by the plugins.
 */
class KWIN_EXPORT SceneFactory : public QObject
{
    Q_OBJECT
public:
    ~SceneFactory() override;

    /**
     * @returns The created Scene, may be @c nullptr.
     */
    virtual Scene *create(QObject *parent = nullptr) const = 0;

protected:
    explicit SceneFactory(QObject *parent);
};

/**
 * The SceneNode class is the base class for all nodes in the scene graph.
 */
class KWIN_EXPORT SceneNode
{
public:
    SceneNode();
    virtual ~SceneNode();

    /**
     * Returns the parent node of this node, or @c null if this is the root node.
     */
    SceneNode *parent() const;

    /**
     * Returns the first child of this node, or @c null if this node doesn't have children.
     */
    SceneNode *firstChild() const;

    /**
     * Returns the last child of this node, or @c null if this node doesn't have children.
     */
    SceneNode *lastChild() const;

    /**
     * Adds the given @p node to the begining of this node's list of children.
     */
    void prependChildNode(SceneNode *node);

    /**
     * Adds the given @p node to the end of this node's list of children.
     */
    void appendChildNode(SceneNode *node);

    /**
     * Removes the given @p node from the list of children of this node.
     */
    void removeChildNode(SceneNode *node);

    /**
     * Inserts @p node to this node's list of children after the reference node @p after.
     */
    void insertChildNodeAfter(SceneNode *node, SceneNode *after);

    /**
     * Inserts @p node to this node's list of children before the reference node @p before.
     */
    void insertChildNodeBefore(SceneNode *node, SceneNode *before);

    /**
     * Returns the Toplevel associated with this node.
     */
    Toplevel *toplevel() const;

    /**
     * Sets the Toplevel associated with this node to @p toplevel.
     */
    void setToplevel(Toplevel *toplevel);

    /**
     *
     */
    WindowQuadList windowQuads() const;

    /**
     *
     */
    void setWindowQuads(const WindowQuadList &windowQuads);

    /**
     * Returns the position of this node relative to the origin of the parent node.
     */
    QPoint position() const;

    /**
     * Sets the position of this node to @p position.
     *
     * The position is relative to the origin of the parent node.
     */
    void setPosition(const QPoint &position);

    /**
     * Returns the position of the origin within the attached texture or image.
     */
    QPoint origin() const;

    /**
     * Sets the origin of this node to @p origin.
     *
     * The origin is relative to the top-left corner of the attached texture.
     */
    void setOrigin(const QPoint &origin);

protected:
    /**
     *
     */
    virtual void updateTexture() = 0;

    /**
     *
     */
    virtual void updateQuads() = 0;

private:
    Toplevel *m_toplevel = nullptr;
    SceneNode *m_parent = nullptr;
    SceneNode *m_nextSibling = nullptr;
    SceneNode *m_previousSibling = nullptr;
    SceneNode *m_firstChild = nullptr;
    SceneNode *m_lastChild = nullptr;
    WindowQuadList m_windowQuads;
    QPoint m_position;
    QPoint m_origin;
};

/**
 * The ShadowSceneNode class represents a server-side drop-shadow in the scene graph.
 *
 * If the Toplevel doesn't have a server-side drop-shadow, then the node tree belonging
 * to the associated scene window will have no shadow nodes.
 */
class KWIN_EXPORT ShadowSceneNode : public SceneNode
{
public:
    ~ShadowSceneNode() override;

    /**
     * Returns the pointer to the server-side drop-shadow.
     */
    Shadow *shadow() const;

protected:
    void updateQuads() override;

private:
    Shadow *m_shadow = nullptr;
};

/**
 * The DecorationSceneNode class represents a server-side decoration in the scene graph.
 *
 * If the Toplevel doesn't have a server-side decoration (for example, if it is client-side
 * decorated), then the node tree that belongs to the associated scene window will have
 * no decoration nodes.
 */
class KWIN_EXPORT DecorationSceneNode : public SceneNode
{
public:

private:
};

/**
 *
 */
class KWIN_EXPORT PlatformSurface
{
public:
    explicit PlatformSurface(SurfaceSceneNode *surfaceNode);
    virtual ~PlatformSurface();

    Toplevel *toplevel() const;

private:
    SurfaceSceneNode *m_surfaceNode;
};

/**
 *
 */
class KWIN_EXPORT X11PlatformSurface : public PlatformSurface
{
public:
    explicit X11PlatformSurface(SurfaceSceneNode *surfaceNode);
    ~X11PlatformSurface() override;

    xcb_pixmap_t pixmap() const;
    QSize size() const;

private:
    xcb_pixmap_t m_pixmap = XCB_PIXMAP_NONE;
    QSize m_size;
};

/**
 *
 */
class KWIN_EXPORT WaylandPlatformSurface : public PlatformSurface
{
public:
    explicit WaylandPlatformSurface(SurfaceSceneNode *surfaceNode);
    ~WaylandPlatformSurface() override;

    KWayland::Server::BufferInterface *buffer() const;
    KWayland::Server::SurfaceInterface *surface() const;

private:
    QPointer<KWayland::Server::SurfaceInterface> m_surface;
    QPointer<KWayland::Server::BufferInterface> m_buffer;
};

/**
 *
 */
class KWIN_EXPORT InternalPlatformSurface : public PlatformSurface
{
public:
    explicit InternalPlatformSurface(SurfaceSceneNode *surfaceNode);
    ~InternalPlatformSurface() override;

    QSharedPointer<QOpenGLFramebufferObject> framebufferObject() const;
    QImage image() const;

private:
    QSharedPointer<QOpenGLFramebufferObject> m_framebufferObject;
    QImage m_image;
};

/**
 * The SurfaceSceneNode class represents a surface with some contents in the scene graph.
 */
class KWIN_EXPORT SurfaceSceneNode : public SceneNode
{
public:
    ~SurfaceSceneNode() override;

    PlatformSurface *platformSurface() const;

private:
    // TODO: Store the associated wl_surface and wl_subsurface objects.
    PlatformSurface *m_platformSurface = nullptr;
};

class Scene::Window
{
public:
    Window(Toplevel *toplevel);
    virtual ~Window();

    // perform the actual painting of the window
    virtual void performPaint(int mask, QRegion region, WindowPaintData data) = 0;
    int x() const;
    int y() const;
    int width() const;
    int height() const;
    QRect geometry() const;
    QPoint pos() const;
    QSize size() const;
    QRect rect() const;
    Toplevel *toplevel() const;
    void setToplevel(Toplevel *toplevel);
    // should the window be painted
    bool isPaintingEnabled() const;
    void resetPaintingEnabled();
    // Flags explaining why painting should be disabled
    enum {
        // Window will not be painted
        PAINT_DISABLED                 = 1 << 0,
        // Window will not be painted because it is deleted
        PAINT_DISABLED_BY_DELETE       = 1 << 1,
        // Window will not be painted because of which desktop it's on
        PAINT_DISABLED_BY_DESKTOP      = 1 << 2,
        // Window will not be painted because it is minimized
        PAINT_DISABLED_BY_MINIMIZE     = 1 << 3,
        // Window will not be painted because it's not on the current activity
        PAINT_DISABLED_BY_ACTIVITY     = 1 << 5
    };
    void enablePainting(int reason);
    void disablePainting(int reason);
    // is the window visible at all
    bool isVisible() const;
    // is the window fully opaque
    bool isOpaque() const;
    // shape of the window
    const QRegion &shape() const;
    QRegion clientShape() const;
    void discardShape();
    // creates initial quad list for the window
    virtual WindowQuadList buildQuads(bool force = false) const;
    void updateShadow(Shadow* shadow);
    const Shadow *shadow() const;
    Shadow *shadow();
    void invalidateQuadsCache();
    ShadowSceneNode *shadowNode() const;
    DecorationSceneNode *decorationNode() const;
    SurfaceSceneNode *surfaceNode() const;
    QVector<SceneNode *> paintOrderNodes() const;

protected:
    ImageFilterType m_filter = ImageFilterFast;

private:
    Toplevel *m_toplevel;
    Shadow *m_shadow = nullptr;
    ShadowSceneNode *m_shadowNode = nullptr;
    DecorationSceneNode *m_decorationNode = nullptr;
    SurfaceSceneNode *m_surfaceNode = nullptr;
    QVector<SceneNode *> m_paintOrderNodes;
    int m_disablePainting = 0;
    mutable QRegion m_shapeRegion;
    mutable bool m_shapeIsValid = false;
    mutable QScopedPointer<WindowQuadList> m_cachedWindowQuads;
    Q_DISABLE_COPY(Window)
};

class Scene::EffectFrame
{
public:
    EffectFrame(EffectFrameImpl* frame);
    virtual ~EffectFrame();
    virtual void render(QRegion region, double opacity, double frameOpacity) = 0;
    virtual void free() = 0;
    virtual void freeIconFrame() = 0;
    virtual void freeTextFrame() = 0;
    virtual void freeSelection() = 0;
    virtual void crossFadeIcon() = 0;
    virtual void crossFadeText() = 0;

protected:
    EffectFrameImpl* m_effectFrame;
};

} // namespace

Q_DECLARE_INTERFACE(KWin::SceneFactory, "org.kde.kwin.Scene")

#endif
