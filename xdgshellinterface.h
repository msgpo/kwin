/*
 * Copyright (C) 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
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

#pragma once

#include <QObject>
#include <QSharedDataPointer>

struct wl_resource;

namespace KWayland
{
namespace Server
{
class Display;
class OutputInterface;
class SeatInterface;
class SurfaceInterface;
}
}

namespace KWin
{

class XdgShellInterfacePrivate;
class XdgSurfaceInterfacePrivate;
class XdgToplevelInterfacePrivate;
class XdgPopupInterfacePrivate;
class XdgPositionerData;
class XdgToplevelInterface;
class XdgPopupInterface;
class XdgSurfaceInterface;

class XdgShellInterface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgShellInterface object with the given wayland display @p display.
     */
    XdgShellInterface(KWayland::Server::Display *display, QObject *parent = nullptr);
    /**
     * Destructs the XdgShellInterface object.
     */
    ~XdgShellInterface() override;

    /**
     * Returns the wayland display of the XdgShellInterface.
     */
    KWayland::Server::Display *display() const;
    /**
     * Sends a ping event to the client with the given xdg-surface @p surface. If the client
     * replies to the event within a reasonable amount of time, pongReceived signal will be
     * emitted.
     */
    quint32 ping(XdgSurfaceInterface *surface);

Q_SIGNALS:
    /**
     * This signal is emitted when a new XdgToplevelInterface object is created.
     */
    void toplevelCreated(XdgToplevelInterface *toplevel);
    /**
     * This signal is emitted when a new XdgPopupInterface object is created.
     */
    void popupCreated(XdgPopupInterface *popup);
    /**
     * This signal is emitted when the client has responded to a ping event with serial @p serial.
     */
    void pongReceived(quint32 serial);
    /**
     * @todo Drop this signal.
     *
     * This signal is emitted when the client has not responded to a ping event with serial
     * @p serial within a reasonable amount of time and the compositor gave up on it.
     */
    void pingTimeout(quint32 serial);
    /**
     * This signal is emitted when the client has not responded to a ping event with serial
     * @p serial within a reasonable amount of time.
     */
    void pingDelayed(quint32 serial);

private:
    QScopedPointer<XdgShellInterfacePrivate> d;
    friend class XdgShellInterfacePrivate;
};

class XdgSurfaceInterface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgSurfaceInterface for the given @p shell and @p surface.
     */
    XdgSurfaceInterface(XdgShellInterface *shell, KWayland::Server::SurfaceInterface *surface,
                        ::wl_resource *resource);
    /**
     * Destructs the XdgSurfaceInterface object.
     */
    ~XdgSurfaceInterface() override;

    /**
     * Returns the XdgToplevelInterface associated with this XdgSurfaceInterface.
     *
     * This method will return @c nullptr if no xdg_toplevel object is associated with this surface.
     */
    XdgToplevelInterface *toplevel() const;
    /**
     * Returns the XdgPopupInterface associated with this XdgSurfaceInterface.
     *
     * This method will return @c nullptr if no xdg_popup object is associated with this surface.
     */
    XdgPopupInterface *popup() const;
    /**
     * Returns the XdgShellInterface associated with this XdgSurfaceInterface.
     */
    XdgShellInterface *shell() const;
    /**
     * Returns the SurfaceInterface assigned to this XdgSurfaceInterface.
     */
    KWayland::Server::SurfaceInterface *surface() const;

    /**
     * Returns the window geometry of the XdgSurfaceInterface.
     *
     * This method will return an invalid QRect if the window geometry is not set by the client.
     */
    QRect windowGeometry() const;

    /**
     * Returns the XdgSurfaceInterface for the specified wayland resource object @p resource.
     */
    static XdgSurfaceInterface *get(::wl_resource *resource);

Q_SIGNALS:
    /**
     * This signal is emitted when a configure event with serial @p serial has been acknowledged.
     */
    void configureAcknowledged(quint32 serial);
    /**
     * This signal is emitted when the window geometry has been changed.
     */
    void windowGeometryChanged(const QRect &rect);

private:
    QScopedPointer<XdgSurfaceInterfacePrivate> d;
    friend class XdgSurfaceInterfacePrivate;
};

class XdgToplevelInterface : public QObject
{
    Q_OBJECT

public:
    enum class State {
        MaximizedHorizontal = 1 << 0,
        MaximizedVertical   = 1 << 1,
        FullScreen          = 1 << 2,
        Resizing            = 1 << 3,
        Activated           = 1 << 4,

        Maximized           = MaximizedHorizontal | MaximizedVertical
    };
    Q_DECLARE_FLAGS(States, State)

    /**
     * Constructs an XdgToplevelInterface for the given xdg-surface @p surface.
     */
    XdgToplevelInterface(XdgSurfaceInterface *surface, ::wl_resource *resource);
    /**
     * Destructs the XdgToplevelInterface object.
     */
    ~XdgToplevelInterface() override;

    /**
     * Returns the XdgShellInterface for this XdgToplevelInterface.
     *
     * This is equivalent to xdgSurface()->shell().
     */
    XdgShellInterface *shell() const;
    /**
     * Returns the XdgSurfaceInterface associated with the XdgToplevelInterface.
     */
    XdgSurfaceInterface *xdgSurface() const;
    /**
     * Returns the SurfaceInterface associated with the XdgToplevelInterface.
     */
    KWayland::Server::SurfaceInterface *surface() const;
    XdgToplevelInterface *parentXdgToplevel() const;

    /**
     * Returns the window title of the toplevel surface.
     */
    QString windowTitle() const;
    /**
     * Returns the window class of the toplevel surface.
     */
    QString windowClass() const;
    /**
     * Returns the minimum window geometry size of the toplevel surface.
     */
    QSize minimumSize() const;
    /**
     * Returns the maximum window geometry size of the toplevel surface.
     */
    QSize maximumSize() const;

    /**
     * Sends a configure event to the client. @p size specifies the new window geometry size. A size
     * of zero means the client should decide its own window dimensions.
     */
    quint32 sendConfigure(const QSize &size, const States &states);
    /**
     * Sends a close event to the client. The client may choose to ignore this request.
     */
    void sendClose();

    /**
     * Returns the XdgToplevelInterface for the specified wayland resource object @p resource.
     */
    static XdgToplevelInterface *get(::wl_resource *resource);

Q_SIGNALS:
    /**
     * This signal is emitted when the xdg-toplevel has commited the initial state and wants to
     * be configured. After initializing the toplevel, you must send a configure event.
     */
    void initializeRequested();
    /**
     * This signal is emitted when the toplevel's title has been changed.
     */
    void windowTitleChanged(const QString &windowTitle);
    /**
     * This signal is emitted when the toplevel's application id has been changed.
     */
    void windowClassChanged(const QString &windowClass);
    void windowMenuRequested(KWayland::Server::SeatInterface *seat, const QPoint &pos, quint32 serial);
    /**
     * This signal is emitted when the toplevel's minimum size has been changed.
     */
    void minimumSizeChanged(const QSize &size);
    /**
     * This signal is emitted when the toplevel's maximum size has been changed.
     */
    void maximumSizeChanged(const QSize &size);
    void moveRequested(KWayland::Server::SeatInterface *seat, quint32 serial);
    void resizeRequested(KWayland::Server::SeatInterface *seat, Qt::Edges edges, quint32 serial);
    /**
     * This signal is emitted when the toplevel surface wants to become maximized.
     */
    void maximizeRequested();
    /**
     * This signal is emitted when the toplevel surface wants to become unmaximized.
     */
    void unmaximizeRequested();
    /**
     * This signal is emitted when the toplevel wants to be shown in the full screen mode.
     */
    void fullscreenRequested(KWayland::Server::OutputInterface *output);
    /**
     * This signal is emitted when the toplevel surface wants to leave the full screen mode.
     */
    void unfullscreenRequested();
    /**
     * This signal is emitted when the toplevel wants to be iconified.
     */
    void minimizeRequested();
    void parentXdgToplevelChanged();

private:
    QScopedPointer<XdgToplevelInterfacePrivate> d;
};

class XdgPositioner
{
public:
    /**
     * Constructs an incomplete XdgPositioner object.
     */
    XdgPositioner();
    /**
     * Constructs a copy of the XdgPositioner object.
     */
    XdgPositioner(const XdgPositioner &other);
    /**
     * Destructs the XdgPositioner object.
     */
    ~XdgPositioner();

    /**
     * Assigns the value of @p other to this XdgPositioner object.
     */
    XdgPositioner &operator=(const XdgPositioner &other);

    /**
     * Returns @c true if the positioner object is complete; otherwise returns @c false.
     *
     * An xdg positioner considered complete if it has a valid size and a valid anchor rect.
     */
    bool isComplete() const;

    /**
     * Returns the set of orientations along which the compositor may slide the popup to ensure
     * that it is entirely inside the compositor's defined "work area."
     */
    Qt::Orientations slideConstraintAdjustments() const;
    /**
     * Returns the set of orientations along which the compositor may flip the popup to ensure
     * that it is entirely inside the compositor's defined "work area."
     */
    Qt::Orientations flipConstraintAdjustments() const;
    /**
     * Returns the set of orientations along which the compositor can resize the popup to ensure
     * that it is entirely inside the compositor's defined "work area."
     */
    Qt::Orientations resizeConstraintAdjustments() const;

    /**
     * Returns the set of edges on the anchor rectangle that the surface should be positioned
     * around.
     */
    Qt::Edges anchorEdges() const;
    /**
     * Returns the direction in which the surface should be positioned, relative to the anchor
     * point of the parent surface.
     */
    Qt::Edges gravityEdges() const;

    /**
     * Returns the window geometry size of the surface that is to be positioned.
     */
    QSize size() const;
    /**
     * Returns the anchor rectangle relative to the upper left corner of the window geometry of
     * the parent surface that the popup should be positioned around.
     */
    QRect anchorRect() const;
    /**
     * Returns the surface position offset relative to the position of the anchor on the anchor
     * rectangle and the anchor on the surface.
     */
    QPoint offset() const;

    /**
     * Returns the current state of the xdg positioner object identified by @p resource.
     */
    static XdgPositioner get(::wl_resource *resource);

private:
    XdgPositioner(const QSharedDataPointer<XdgPositionerData> &data);
    QSharedDataPointer<XdgPositionerData> d;
};

class XdgPopupInterface : public QObject
{
    Q_OBJECT

public:
    XdgPopupInterface(XdgSurfaceInterface *surface, XdgSurfaceInterface *parentSurface,
                      const XdgPositioner &positioner, ::wl_resource *resource);
    /**
     * Destructs the XdgPopupInterface object.
     */
    ~XdgPopupInterface() override;

    XdgShellInterface *shell() const;
    /**
     * Returns the parent XdgSurfaceInterface.
     *
     * This method may return @c nullptr, in which case the parent xdg-surface must be specified
     * using "some other protocol", before commiting the initial state.
     */
    XdgSurfaceInterface *parentXdgSurface() const;
    /**
     * Returns the XdgSurfaceInterface associated with the XdgPopupInterface.
     */
    XdgSurfaceInterface *xdgSurface() const;
    /**
     * Returns the SurfaceInterface associated with the XdgPopupInterface.
     */
    KWayland::Server::SurfaceInterface *surface() const;
    /**
     * Returns the XdgPositioner assigned to this XdgPopupInterface.
     */
    XdgPositioner positioner() const;

    /**
     * Sends a configure event to the client and returns the serial number of the event.
     */
    quint32 sendConfigure(const QRect &rect);
    /**
     * Sends a popup done event to the client.
     */
    void sendPopupDone();

    /**
     * Returns the XdgPopupInterface for the specified wayland resource object @p resource.
     */
    static XdgPopupInterface *get(::wl_resource *resource);

Q_SIGNALS:
    /**
     * This signal is emitted when the xdg-popup has commited the initial state and wants to
     * be configured. After initializing the popup, you must send a configure event.
     */
    void initializeRequested();
    void grabRequested(KWayland::Server::SeatInterface *seat, quint32 serial);

private:
    QScopedPointer<XdgPopupInterfacePrivate> d;
};

} // namespace KWin

Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::XdgToplevelInterface::States)
