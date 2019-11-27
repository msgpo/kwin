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
class Deleted;
class EffectFrameImpl;
class EffectWindowImpl;
class OverlayWindow;
class Shadow;
class WindowPixmap;

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
    virtual qint64 paint(QRegion damage, QList<Toplevel *> windows) = 0;

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
     * Replaces the Toplevel with an instance of the Deleted class.
     *
     * @param toplevel The Toplevel instance about to be replaced
     * @param deleted The replacement
     */
    void replaceToplevel(Toplevel *toplevel, Deleted *deleted);

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

Q_SIGNALS:
    void frameRendered();
    void resetCompositing();

protected:
    virtual Window *createWindow(Toplevel *toplevel) = 0;
    void createStackingOrder(QList<Toplevel *> toplevels);
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
 * The SceneNode class is a base class for scene nodes.
 *
 * A window may be composed of a bunch of elements, for example, a server-side drop-shadow,
 * a server-side decoration, and the client surface(s) with the actual contents. All those
 * components are represented with scene nodes.
 *
 * The scene nodes form a tree, with RootSceneNode being at the top of it. The scene window
 * always has a root node and at least one surface node. The shadow node and the decoration
 * node may be not present.
 *
 * The child nodes are rendered in the order they were added to the scene, from left to
 * right, from parent to child.
 */
class KWIN_EXPORT SceneNode
{
public:
    virtual ~SceneNode();

    enum NodeType {
        RootNodeType,
        ShadowNodeType,
        DecorationNodeType,
        SurfaceNodeType,
    };

    /**
     * Returns the internal type of this node.
     */
    NodeType type() const;

    /**
     * Returns the parent node of this node, or @c null if that's the root node.
     */
    SceneNode *parent() const;

    /**
     * Returns the first child of this node, or @c null if there are no children.
     */
    SceneNode *firstChild() const;

    /**
     * Returns the last child of this node, or @c null if there are no children.
     */
    SceneNode *lastChild() const;

    /**
     * Returns the node immediately preceding this node in the parent's list of children.
     */
    SceneNode *previousSibling() const;

    /**
     * Returns the node immediately following this node in the parent's list of children.
     */
    SceneNode *nextSibling() const;

    /**
     * Returns all nodes stored in the subtree of this node.
     *
     * Note that the returned list of nodes is DFS ordered.
     */
    QVector<SceneNode *> subtreeNodes() const;

    /**
     * Adds the given @p node to the beginning of this node's list of children.
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
     * Returns the Scene::Window associated with this scene node.
     */
    Scene::Window *sceneWindow() const;

    /**
     * Returns the Toplevel associated with this scene node.
     */
    Toplevel *toplevel() const;

    /**
     * Sets the Toplevel associated with this scene node to @p toplevel.
     *
     * The child nodes will be reparented to @p toplevel together with this node.
     */
    void setToplevel(Toplevel *toplevel);

    /**
     * Returns the list of window quads that specify the area occupied by the node.
     */
    WindowQuadList windowQuads() const;

    /**
     * Sets the list of window quads to @p windowQuads.
     *
     * The window vertices must be relative to the top-left corner of the parent node.
     */
    void setWindowQuads(const WindowQuadList &windowQuads);

    /**
     * Returns the position of this node, relative to the top-left corner of the parent node.
     */
    QPoint position() const;

    /**
     * Sets the position of this node to @p position, relative to the parent node.
     */
    void setPosition(const QPoint &position);

    /**
     * Returns the position of this node relative to the top-left corner of the frame.
     */
    QPoint combinedPosition() const;

    /**
     * Sets the position of this node relative to the top-left corner of the frame.
     */
    void setCombinedPosition(const QPoint &position);

    /**
     * Returns the dimensions of the area occupied by this node in device independent pixels.
     */
    QSize size() const;

    /**
     * Sets the dimensions of the area occupied by this node to @p size.
     */
    void setSize(const QSize &size);

    /**
     * Returns the internal geometry of the node.
     */
    QRect rect() const;

    QRegion shape() const;

    void setShape(const QRegion &shape);

    /**
     * Returns the id of this node. The node id is used to identify window quads.
     */
    int id() const;

    /**
     * Sets the id of this node to @p id. The id must be the node's painting order index.
     */
    void setId(int id);

    /**
     * Returns @c true if the node is visible.
     */
    bool isVisible() const;

    /**
     * Sets the visible status of the node to @p visible.
     */
    void setVisible(bool visible);

    void forEachChild(std::function<void (SceneNode *child)> func);

    virtual void updateChildren();
    virtual void updateQuads();
    virtual void updateTexture();

protected:
    explicit SceneNode(Toplevel *toplevel, NodeType nodeType);

private:
    Toplevel *m_toplevel = nullptr;
    SceneNode *m_parent = nullptr;
    SceneNode *m_firstChild = nullptr;
    SceneNode *m_lastChild = nullptr;
    SceneNode *m_previousSibling = nullptr;
    SceneNode *m_nextSibling = nullptr;
    WindowQuadList m_windowQuads;
    QRegion m_shape;
    QSize m_size;
    QPoint m_position;
    QPoint m_combinedPosition;
    NodeType m_nodeType;
    bool m_isVisible = false;
    int m_id = -1;
};

/**
 * The ShadowSceneNode class represents a server-side drop-shadow in the scene.
 */
class KWIN_EXPORT ShadowSceneNode : public SceneNode
{
public:
    explicit ShadowSceneNode(Toplevel *toplevel);

    /**
     * Returns the internal server-side drop-shadow.
     */
    Shadow *shadow() const;

    /**
     * Sets the internal server-side drop-shadow to @p shadow.
     */
    void setShadow(Shadow *shadow);

    void updateQuads() override final;

private:
    QScopedPointer<Shadow> m_shadow;
};

/**
 * The DecorationSceneNode class represents a server-side decoration in the scene.
 */
class KWIN_EXPORT DecorationSceneNode : public SceneNode
{
public:
    explicit DecorationSceneNode(Toplevel *toplevel);

    void updateQuads() override final;
};

/**
 * The SurfaceSceneNode class represents a surface with pixel data in the scene.
 */
class KWIN_EXPORT SurfaceSceneNode : public SceneNode
{
public:
    explicit SurfaceSceneNode(Toplevel *toplevel);

    /**
     * Returns the wayland surface associated with the SurfaceSceneNode.
     *
     * Note that this method may return @c null if the node represents an X11 pixmap.
     */
    KWayland::Server::SurfaceInterface *surface() const;

    /**
     * Sets the wayland surface to @p surface.
     */
    void setSurface(KWayland::Server::SurfaceInterface *surface);

    void updateChildren() override final;
    void updateQuads() override final;

private:
    SceneNode *fetchChildNode(int childIndex, SceneNode *currentNode) const;

    QPointer<KWayland::Server::SurfaceInterface> m_surface;
};

/**
 * The RootSceneNode class represents the root node of the scene window.
 */
class KWIN_EXPORT RootSceneNode : public SceneNode
{
public:
    explicit RootSceneNode(Toplevel *toplevel);

    /**
     * Returns the ShadowSceneNode that represents the server-side drop-shadow.
     *
     * Note that this method may return @c null.
     */
    ShadowSceneNode *shadowNode() const;

    /**
     * Returns the DecorationSceneNode that represents the server-side decoration.
     *
     * Note that this method may return @c null.
     */
    DecorationSceneNode *decorationNode() const;

    /**
     * Returns the SurfaceSceneNode that represents the main surface.
     */
    SurfaceSceneNode *surfaceNode() const;

    void updateChildren() override final;

private:
    ShadowSceneNode *m_shadowNode = nullptr;
    DecorationSceneNode *m_decorationNode = nullptr;
    SurfaceSceneNode *m_surfaceNode = nullptr;
};

/**
 * @brief The SceneNodeUpdater class
 */
class SceneNodeUpdater
{
public:
    explicit SceneNodeUpdater(Scene::Window *window);

    void update(int dirtyAttributes);

private:
    void enterShadowNode(ShadowSceneNode *shadowNode);
    void leaveShadowNode(ShadowSceneNode *shadowNode);
    void enterDecorationNode(DecorationSceneNode *decorationNode);
    void leaveDecorationNode(DecorationSceneNode *decorationNode);
    void enterSurfaceNode(SurfaceSceneNode *surfaceNode);
    void leaveSurfaceNode(SurfaceSceneNode *surfaceNode);

    void visitNode(SceneNode *node);
    void visitChildren(SceneNode *node);

    Scene::Window *m_window = nullptr;
    int m_dirtyAttributes = 0;
    int m_lastNodeId = 0;
};

// The base class for windows representations in composite backends
class Scene::Window
{
public:
    Window(Toplevel* c);
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
    // access to the internal window class
    // TODO eventually get rid of this
    Toplevel* window() const;
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
    void updateToplevel(Toplevel* c);
    void updateShadow(Shadow* shadow);
    const Shadow* shadow() const;
    Shadow* shadow();

    enum AttributeFlag {
        Geometry = 1 << 1,
        Shape = 1 << 2,
        Children = 1 << 3,
        Quads = 1 << 4,
        Pixmap = 1 << 5,
        Visibility = 1 << 6,

        GenericUpdateMask = Geometry | Shape | Children | Visibility,
        QuadsUpdateMask = GenericUpdateMask | Quads,
        AllUpdateMask = GenericUpdateMask | Quads | Pixmap,
    };

    /**
     * Notifies the scene window that the given @p attributes of some node must be updated.
     *
     * The attributes are not updated immediately but the next time the scene window is painted.
     */
    void markDirtyAttributes(int attributes);

    void updateDirtyAttributes();

    /**
     * Returns the root scene node.
     */
    RootSceneNode *rootNode() const;

    /**
     * @brief windowQuads
     * @return
     */
    WindowQuadList windowQuads() const;

    /**
     * Returns the scene nodes in the painting order, from left to right.
     */
    QVector<SceneNode *> paintOrderNodes() const;

    /**
     * @brief Factory method to create a WindowPixmap.
     *
     * The inheriting classes need to implement this method to create a new instance of
     * their WindowPixmap subclass. Do not use WindowPixmap::create on the created instance.
     * The Scene will take care of that.
     */
    virtual WindowPixmap *createWindowPixmap() = 0;

    /**
     * Factory method to create an instance of the ShadowSceneNode class.
     *
     * The inheriting classes need to implement this method to create a new instance of
     * their ShadowSceneNode subclasses.
     */
    virtual ShadowSceneNode *createShadowNode() = 0;

    /**
     * Factory method to create an instance of the DecorationSceneNode class.
     *
     * The inheriting classes need to implement this method to create a new instance of
     * their DecorationSceneNode subclasses.
     */
    virtual DecorationSceneNode *createDecorationNode() = 0;

    /**
     * Factory method to create an instance of the SurfaceSceneNode class.
     *
     * The inheriting classes need to implement this method to create a new instance of
     * their SurfaceSceneNode subclasses.
     */
    virtual SurfaceSceneNode *createSurfaceNode() = 0;

protected:
    Toplevel* toplevel;
    ImageFilterType filter = ImageFilterFast;
    Shadow *m_shadow = nullptr;

private:
    SceneNodeUpdater *m_nodeUpdater;
    QVector<SceneNode *> m_paintOrderNodes;
    RootSceneNode *m_rootNode = nullptr;
    WindowQuadList m_windowQuads;
    int m_dirtyAttributes = AllUpdateMask;
    int m_disablePainting = 0;
    Q_DISABLE_COPY(Window)
};

/**
 * @brief Wrapper for a pixmap of the Scene::Window.
 *
 * This class encapsulates the functionality to get the pixmap for a window. When initialized the pixmap is not yet
 * mapped to the window and isValid will return @c false. The pixmap mapping to the window can be established
 * through @ref create. If it succeeds isValid will return @c true, otherwise it will keep in the non valid
 * state and it can be tried to create the pixmap mapping again (e.g. in the next frame).
 *
 * This class is not intended to be updated when the pixmap is no longer valid due to e.g. resizing the window.
 * Instead a new instance of this class should be instantiated. The idea behind this is that a valid pixmap does not
 * get destroyed, but can continue to be used. To indicate that a newer pixmap should in generally be around, one can
 * use markAsDiscarded.
 *
 * This class is intended to be inherited for the needs of the compositor backends which need further mapping from
 * the native pixmap to the respective rendering format.
 */
class KWIN_EXPORT WindowPixmap
{
public:
    virtual ~WindowPixmap();
    /**
     * @brief Tries to create the mapping between the Window and the pixmap.
     *
     * In case this method succeeds in creating the pixmap for the window, isValid will return @c true otherwise
     * @c false.
     *
     * Inheriting classes should re-implement this method in case they need to add further functionality for mapping the
     * native pixmap to the rendering format.
     */
    virtual void create();
    /**
     * @return @c true if the pixmap has been created and is valid, @c false otherwise
     */
    virtual bool isValid() const;
    /**
     * @return The native X11 pixmap handle
     */
    xcb_pixmap_t pixmap() const;
    /**
     * @return The Wayland BufferInterface for this WindowPixmap.
     */
    QPointer<KWayland::Server::BufferInterface> buffer() const;
    const QSharedPointer<QOpenGLFramebufferObject> &fbo() const;
    QImage internalImage() const;
    /**
     * @brief Returns the Toplevel this WindowPixmap belongs to.
     * Note: the Toplevel can change over the lifetime of the WindowPixmap in case the Toplevel is copied to Deleted.
     */
    Toplevel *toplevel() const;

    /**
     * @returns the surface this WindowPixmap references, might be @c null.
     */
    KWayland::Server::SurfaceInterface *surface() const;

protected:
    explicit WindowPixmap(Scene::Window *window);

    /**
     * @return The Window this WindowPixmap belongs to
     */
    Scene::Window *window();

    /**
     * Should be called by the implementing subclasses when the Wayland Buffer changed and needs
     * updating.
     */
    virtual void updateBuffer();

private:
    Scene::Window *m_window;
    xcb_pixmap_t m_pixmap = XCB_PIXMAP_NONE;
    QPointer<KWayland::Server::BufferInterface> m_buffer;
    QPointer<KWayland::Server::SurfaceInterface> m_surface;
    QSharedPointer<QOpenGLFramebufferObject> m_fbo;
    QImage m_internalImage;
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

inline
int Scene::Window::x() const
{
    return toplevel->x();
}

inline
int Scene::Window::y() const
{
    return toplevel->y();
}

inline
int Scene::Window::width() const
{
    return toplevel->width();
}

inline
int Scene::Window::height() const
{
    return toplevel->height();
}

inline
QRect Scene::Window::geometry() const
{
    return toplevel->frameGeometry();
}

inline
QSize Scene::Window::size() const
{
    return toplevel->size();
}

inline
QPoint Scene::Window::pos() const
{
    return toplevel->pos();
}

inline
QRect Scene::Window::rect() const
{
    return toplevel->rect();
}

inline
Toplevel* Scene::Window::window() const
{
    return toplevel;
}

inline
void Scene::Window::updateToplevel(Toplevel* c)
{
    toplevel = c;
}

inline
const Shadow* Scene::Window::shadow() const
{
    return m_shadow;
}

inline
Shadow* Scene::Window::shadow()
{
    return m_shadow;
}

inline
QPointer<KWayland::Server::BufferInterface> WindowPixmap::buffer() const
{
    return m_buffer;
}

inline
const QSharedPointer<QOpenGLFramebufferObject> &WindowPixmap::fbo() const
{
    return m_fbo;
}

inline
QImage WindowPixmap::internalImage() const
{
    return m_internalImage;
}

inline
Toplevel* WindowPixmap::toplevel() const
{
    return m_window->window();
}

inline
xcb_pixmap_t WindowPixmap::pixmap() const
{
    return m_pixmap;
}

} // namespace

Q_DECLARE_INTERFACE(KWin::SceneFactory, "org.kde.kwin.Scene")

#endif
