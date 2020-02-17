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

class XdgShellV6InterfacePrivate;
class XdgSurfaceV6InterfacePrivate;
class XdgToplevelV6InterfacePrivate;
class XdgPopupV6InterfacePrivate;
class XdgPositionerV6Data;
class XdgToplevelV6Interface;
class XdgPopupV6Interface;
class XdgSurfaceV6Interface;

class XdgShellV6Interface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgShellV6Interface with the given wayland display @p display.
     */
    XdgShellV6Interface(KWayland::Server::Display *display, QObject *parent = nullptr);
    /**
     * Destructs the XdgShellV6Interface object.
     */
    ~XdgShellV6Interface() override;

    /**
     * Returns the wayland display of the XdgShellV6Interface.
     */
    KWayland::Server::Display *display() const;
    /**
     * Sends a ping event to the client with the given xdg-surface @p surface. If the client
     * replies to the event within a reasonable amount of time, pong signal will be emitted.
     */
    quint32 ping(XdgSurfaceV6Interface *surface);

Q_SIGNALS:
    /**
     * This signal is emitted when a new XdgToplevelV6Interface object is created.
     */
    void toplevelCreated(XdgToplevelV6Interface *toplevel);
    /**
     * This signal is emitted when a new XdgPopupV6Interface object is created.
     */
    void popupCreated(XdgPopupV6Interface *popup);
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
    QScopedPointer<XdgShellV6InterfacePrivate> d;
    friend class XdgShellV6InterfacePrivate;
};

class XdgSurfaceV6Interface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgSurfaceV6Interface with the specified @p shell and @p surface.
     */
    XdgSurfaceV6Interface(XdgShellV6Interface *shell, KWayland::Server::SurfaceInterface *surface,
                          ::wl_resource *resource);
    /**
     * Destructs the XdgSurfaceV6Interface object.
     */
    ~XdgSurfaceV6Interface() override;

    /**
     * Returns the XdgToplevelV6Interface associated with this XdgSurfaceV6Interface.
     *
     * This method will return @c nullptr if no zxdg_toplevel_v6 object is associated with this surface.
     */
    XdgToplevelV6Interface *toplevel() const;
    /**
     * Returns the XdgPopupV6Interface associated with this XdgSurfaceV6Interface.
     *
     * This method will return @c nullptr if no zxdg_popup_v6 object is associated with this surface.
     */
    XdgPopupV6Interface *popup() const;
    /**
     * Returns the XdgShellV6Interface associated with this XdgSurfaceV6Interface.
     */
    XdgShellV6Interface *shell() const;
    /**
     * Returns the SurfaceInterface assigned to this XdgSurfaceV6Interface.
     */
    KWayland::Server::SurfaceInterface *surface() const;

    /**
     * Returns the window geometry of the XdgSurfaceV6Interface.
     *
     * This method will return an invalid QRect if the window geometry is not set by the client.
     */
    QRect windowGeometry() const;

    /**
     * Returns the XdgSurfaceV6Interface for the specified wayland resource object @p resource.
     */
    static XdgSurfaceV6Interface *get(::wl_resource *resource);

Q_SIGNALS:
    void configureAcknowledged(quint32 serial);
    void windowGeometryChanged(const QRect &rect);

private:
    QScopedPointer<XdgSurfaceV6InterfacePrivate> d;
    friend class XdgSurfaceV6InterfacePrivate;
};

class XdgToplevelV6Interface : public QObject
{
    Q_OBJECT

public:
    enum class State {
        MaximizedHorizontal = 1 << 0,
        MaximizedVertical   = 1 << 1,
        FullScreen          = 1 << 2,
        Resizing            = 1 << 3,
        Activated           = 1 << 4,

        Maximized           = MaximizedVertical | MaximizedHorizontal
    };
    Q_DECLARE_FLAGS(States, State)

    /**
     * Constructs an XdgToplevelV6Interface with the given xdg-surface @p surface.
     */
    XdgToplevelV6Interface(XdgSurfaceV6Interface *surface, ::wl_resource *resource);
    /**
     * Destructs the XdgToplevelV6Interface object.
     */
    ~XdgToplevelV6Interface() override;

    /**
     * Returns the XdgShellInterface for this XdgToplevelV6Interface.
     *
     * This is equivalent to xdgSurface()->shell().
     */
    XdgShellV6Interface *shell() const;
    /**
     * Returns the XdgSurfaceV6Interface associated with the XdgToplevelV6Interface.
     */
    XdgSurfaceV6Interface *xdgSurface() const;
    /**
     * Returns the SurfaceInterface associated with the XdgToplevelV6Interface.
     */
    KWayland::Server::SurfaceInterface *surface() const;
    XdgToplevelV6Interface *parentXdgToplevel() const;

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
     * Returns the XdgToplevelV6Interface for the specified wayland resource object @p resource.
     */
    static XdgToplevelV6Interface *get(::wl_resource *resource);

Q_SIGNALS:
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
    QScopedPointer<XdgToplevelV6InterfacePrivate> d;
};

class XdgPositionerV6
{
public:
    /**
     * Constructs an incomplete XdgPositionerV6 object.
     */
    XdgPositionerV6();
    /**
     * Constructs a copy of the XdgPositionerV6 object.
     */
    XdgPositionerV6(const XdgPositionerV6 &other);
    /**
     * Destructs the XdgPositionerV6 object.
     */
    ~XdgPositionerV6();

    /**
     * Assigns the value of @p other to this XdgPositionerV6 object.
     */
    XdgPositionerV6 &operator=(const XdgPositionerV6 &other);

    /**
     * Returns @c true if the XdgPositionerV6 is complete; otherwise returns @c false.
     *
     * An xdg positioner is considered complete if it has a valid size and a valid anchor rect.
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
     * Returns the set of edges on the anchor rectangle that the surface should be positioned around.
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
    static XdgPositionerV6 get(::wl_resource *resource);

private:
    XdgPositionerV6(const QSharedDataPointer<XdgPositionerV6Data> &data);
    QSharedDataPointer<XdgPositionerV6Data> d;
};

class XdgPopupV6Interface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgPopupV6Interface with the given xdg-surface @p surface.
     */
    XdgPopupV6Interface(XdgSurfaceV6Interface *surface, XdgSurfaceV6Interface *parentSurface,
                        const XdgPositionerV6 &positioner, ::wl_resource *resource);
    /**
     * Destructs the XdgPopupV6Interface object.
     */
    ~XdgPopupV6Interface() override;

    XdgShellV6Interface *shell() const;
    /**
     * Returns the parent XdgSurfaceV6Interface.
     */
    XdgSurfaceV6Interface *parentXdgSurface() const;
    /**
     * Returns the XdgSurfaceV6Interface associated with the XdgPopupV6Interface.
     */
    XdgSurfaceV6Interface *xdgSurface() const;
    /**
     * Returns the SurfaceInterface associated with the XdgPopupV6Interface.
     */
    KWayland::Server::SurfaceInterface *surface() const;
    /**
     * Returns the XdgPositionerV6 assigned to this XdgPopupV6Interface.
     */
    XdgPositionerV6 positioner() const;

    /**
     * Sends a configure event to the client and returns the serial number of the event.
     */
    quint32 sendConfigure(const QRect &rect);
    /**
     * Sends a popup done event to the client.
     */
    void sendPopupDone();

    /**
     * Returns the XdgPopupV6Interface for the specified wayland resource object @p resource.
     */
    static XdgPopupV6Interface *get(::wl_resource *resource);

Q_SIGNALS:
    void initializeRequested();
    void grabRequested(KWayland::Server::SeatInterface *seat, quint32 serial);

private:
    QScopedPointer<XdgPopupV6InterfacePrivate> d;
};

} // namespace KWin

Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::XdgToplevelV6Interface::States)
