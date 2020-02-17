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

#include "xdgshellv6interface.h"
#include "xdgshellv6interface_p.h"

#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>
#include <KWayland/Server/seat_interface.h>

#include <QTimer>

using namespace KWayland::Server;

namespace KWin
{

// TODO: Reset the surface when it becomes unmapped.

XdgShellV6InterfacePrivate::XdgShellV6InterfacePrivate(XdgShellV6Interface *shell)
    : q(shell)
{
}

static wl_client *clientFromXdgSurface(XdgSurfaceV6Interface *surface)
{
    return XdgSurfaceV6InterfacePrivate::get(surface)->resource()->client();
}

void XdgShellV6InterfacePrivate::registerXdgSurface(XdgSurfaceV6Interface *surface)
{
    xdgSurfaces.insert(clientFromXdgSurface(surface), surface);
}

void XdgShellV6InterfacePrivate::unregisterXdgSurface(XdgSurfaceV6Interface *surface)
{
    xdgSurfaces.remove(clientFromXdgSurface(surface), surface);
}

/**
 * @todo Whether the ping is delayed or has timed out is out of domain of the XdgShellV6Interface.
 * Such matter must be handled somewhere else, e.g. XdgToplevelV6Client, not here!
 */
void XdgShellV6InterfacePrivate::registerPing(quint32 serial)
{
    QTimer *timer = new QTimer(q);
    timer->setInterval(1000);
    QObject::connect(timer, &QTimer::timeout, q, [this, serial, attempt = 0]() mutable {
        ++attempt;
        if (attempt == 1) {
            emit q->pingDelayed(serial);
            return;
        }
        emit q->pingTimeout(serial);
        delete pings.take(serial);
    });
    pings.insert(serial, timer);
    timer->start();
}

XdgShellV6InterfacePrivate *XdgShellV6InterfacePrivate::get(XdgShellV6Interface *shell)
{
    return shell->d.data();
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_destroy(Resource *resource)
{
    if (xdgSurfaces.contains(resource->client())) {
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_DEFUNCT_SURFACES,
                               "zxdg_shell_v6 was destroyed before children");
        return;
    }
    wl_resource_destroy(resource->handle);
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_create_positioner(Resource *resource, uint32_t id)
{
    wl_resource *positionerResource = wl_resource_create(resource->client(), &zxdg_positioner_v6_interface,
                                                         wl_resource_get_version(resource->handle), id);
    new XdgPositionerV6Private(positionerResource);
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id,
                                                               ::wl_resource *surfaceResource)
{
    SurfaceInterface *surface = SurfaceInterface::get(surfaceResource);

    if (surface->buffer()) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_UNCONFIGURED_BUFFER,
                               "zxdg_surface_v6 must not have a buffer at creation");
        return;
    }

    wl_resource *xdgSurfaceResource = wl_resource_create(resource->client(), &zxdg_surface_v6_interface,
                                                         wl_resource_get_version(resource->handle), id);

    XdgSurfaceV6Interface *xdgSurface = new XdgSurfaceV6Interface(q, surface, xdgSurfaceResource);
    registerXdgSurface(xdgSurface);
}

void XdgShellV6InterfacePrivate::zxdg_shell_v6_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource)
    if (QTimer *timer = pings.take(serial)) {
        delete timer;
        emit q->pongReceived(serial);
    }
}

XdgShellV6Interface::XdgShellV6Interface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new XdgShellV6InterfacePrivate(this))
{
    d->display = display;
    d->init(*display, 1);
}

XdgShellV6Interface::~XdgShellV6Interface()
{
}

Display *XdgShellV6Interface::display() const
{
    return d->display;
}

quint32 XdgShellV6Interface::ping(XdgSurfaceV6Interface *surface)
{
    ::wl_client *client = clientFromXdgSurface(surface);

    XdgShellV6InterfacePrivate::Resource *clientResource = d->resourceMap().value(client);
    if (!clientResource)
        return 0;

    quint32 serial = d->display->nextSerial();
    d->send_ping(clientResource->handle, serial);
    d->registerPing(serial);

    return serial;
}

XdgSurfaceV6InterfacePrivate::XdgSurfaceV6InterfacePrivate(XdgSurfaceV6Interface *xdgSurface)
    : q(xdgSurface)
{
}

void XdgSurfaceV6InterfacePrivate::commit()
{
    if (current.windowGeometry != next.windowGeometry) {
        current.windowGeometry = next.windowGeometry;
        emit q->windowGeometryChanged(current.windowGeometry);
    }
}

XdgSurfaceV6InterfacePrivate *XdgSurfaceV6InterfacePrivate::get(XdgSurfaceV6Interface *surface)
{
    return surface->d.data();
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    XdgShellV6InterfacePrivate::get(shell)->unregisterXdgSurface(q);
    delete q;
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_destroy(Resource *resource)
{
    if (toplevel || popup) {
        qWarning() << "Tried to destroy zxdg_surface_v6 before its role object";
    }
    wl_resource_destroy(resource->handle);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id)
{
    if (SurfaceRole::get(surface)) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_ALREADY_CONSTRUCTED,
                               "zxdg_surface_v6 has already been constructured");
        return;
    }

    wl_resource *toplevelResource = wl_resource_create(resource->client(), &zxdg_toplevel_v6_interface,
                                                       wl_resource_get_version(resource->handle), id);

    toplevel = new XdgToplevelV6Interface(q, toplevelResource);
    emit shell->toplevelCreated(toplevel);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_get_popup(Resource *resource, uint32_t id,
                                                             ::wl_resource *parentResource,
                                                             ::wl_resource *positionerResource)
{
    if (SurfaceRole::get(surface)) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_ALREADY_CONSTRUCTED,
                               "zxdg_surface_v6 has already been constructured");
        return;
    }

    XdgPositionerV6 positioner = XdgPositionerV6::get(positionerResource);
    if (!positioner.isComplete()) {
        wl_resource_post_error(resource->handle, ZXDG_SHELL_V6_ERROR_INVALID_POSITIONER,
                               "zxdg_positioner_v6 is incomplete");
        return;
    }

    XdgSurfaceV6Interface *parentSurface = XdgSurfaceV6Interface::get(parentResource);
    if (!parentSurface) {
        wl_resource_post_error(resource->handle, -1, "parent surface is not set");
        return;
    }

    wl_resource *popupResource = wl_resource_create(resource->client(), &zxdg_popup_v6_interface,
                                                    wl_resource_get_version(resource->handle), id);

    popup = new XdgPopupV6Interface(q, parentSurface, positioner, popupResource);
    emit shell->popupCreated(popup);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_set_window_geometry(Resource *resource,
                                                                       int32_t x, int32_t y,
                                                                       int32_t width, int32_t height)
{
    if (!toplevel && !popup) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_NOT_CONSTRUCTED,
                               "zxdg_surface_v6 must have a role");
        return;
    }

    if (width < 1 || height < 1) {
        wl_resource_post_error(resource->handle, -1, "invalid window geometry size");
        return;
    }

    next.windowGeometry = QRect(x, y, width, height);
}

void XdgSurfaceV6InterfacePrivate::zxdg_surface_v6_ack_configure(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource)
    emit q->configureAcknowledged(serial);
}

XdgSurfaceV6Interface::XdgSurfaceV6Interface(XdgShellV6Interface *shell, SurfaceInterface *surface,
                                             ::wl_resource *resource)
    : d(new XdgSurfaceV6InterfacePrivate(this))
{
    d->shell = shell;
    d->surface = surface;
    d->init(resource);
}

XdgSurfaceV6Interface::~XdgSurfaceV6Interface()
{
}

XdgToplevelV6Interface *XdgSurfaceV6Interface::toplevel() const
{
    return d->toplevel;
}

XdgPopupV6Interface *XdgSurfaceV6Interface::popup() const
{
    return d->popup;
}

XdgShellV6Interface *XdgSurfaceV6Interface::shell() const
{
    return d->shell;
}

SurfaceInterface *XdgSurfaceV6Interface::surface() const
{
    return d->surface;
}

QRect XdgSurfaceV6Interface::windowGeometry() const
{
    return d->current.windowGeometry;
}

XdgSurfaceV6Interface *XdgSurfaceV6Interface::get(::wl_resource *resource)
{
    if (auto surface = QtWaylandServer::zxdg_surface_v6::Resource::fromResource(resource)) {
        return static_cast<XdgSurfaceV6InterfacePrivate *>(surface->object())->q;
    }
    return nullptr;
}

XdgToplevelV6InterfacePrivate::XdgToplevelV6InterfacePrivate(XdgToplevelV6Interface *toplevel,
                                                             XdgSurfaceV6Interface *surface)
    : SurfaceRole(surface->surface())
    , q(toplevel)
    , xdgSurface(surface)
{
}

void XdgToplevelV6InterfacePrivate::commit()
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (xdgSurfacePrivate->isConfigured) {
        xdgSurfacePrivate->commit();
    } else {
        emit q->initializeRequested();
        return;
    }

    if (current.minimumSize != next.minimumSize) {
        current.minimumSize = next.minimumSize;
        emit q->minimumSizeChanged(current.minimumSize);
    }
    if (current.maximumSize != next.maximumSize) {
        current.maximumSize = next.maximumSize;
        emit q->maximumSizeChanged(current.maximumSize);
    }
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete q;
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_parent(Resource *resource,
                                                                ::wl_resource *parentResource)
{
    Q_UNUSED(resource)
    XdgToplevelV6Interface *parent = XdgToplevelV6Interface::get(parentResource);
    if (parentXdgToplevel == parent) {
        return;
    }
    parentXdgToplevel = parent;
    emit q->parentXdgToplevelChanged();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_title(Resource *resource, const QString &title)
{
    Q_UNUSED(resource)
    if (windowTitle == title) {
        return;
    }
    windowTitle = title;
    emit q->windowTitleChanged(title);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_app_id(Resource *resource, const QString &app_id)
{
    Q_UNUSED(resource)
    if (windowClass == app_id) {
        return;
    }
    windowClass = app_id;
    emit q->windowClassChanged(app_id);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_show_window_menu(Resource *resource,
                                                                      ::wl_resource *seatResource,
                                                                      uint32_t serial, int32_t x, int32_t y)
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (!xdgSurfacePrivate->isConfigured) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_NOT_CONSTRUCTED,
                               "surface has not been configured yet");
        return;
    }

    SeatInterface *seat = SeatInterface::get(seatResource);
    emit q->windowMenuRequested(seat, QPoint(x, y), serial);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_move(Resource *resource, ::wl_resource *seatResource, uint32_t serial)
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (!xdgSurfacePrivate->isConfigured) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_NOT_CONSTRUCTED,
                               "surface has not been configured yet");
        return;
    }

    SeatInterface *seat = SeatInterface::get(seatResource);
    emit q->moveRequested(seat, serial);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_resize(Resource *resource,
                                                            ::wl_resource *seatResource,
                                                            uint32_t serial, uint32_t zxdgEdges)
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (!xdgSurfacePrivate->isConfigured) {
        wl_resource_post_error(resource->handle, ZXDG_SURFACE_V6_ERROR_NOT_CONSTRUCTED,
                               "surface has not been configured yet");
        return;
    }

    SeatInterface *seat = SeatInterface::get(seatResource);

    Qt::Edges edges;
    if (zxdgEdges & ZXDG_TOPLEVEL_V6_RESIZE_EDGE_TOP) {
        edges |= Qt::TopEdge;
    }
    if (zxdgEdges & ZXDG_TOPLEVEL_V6_RESIZE_EDGE_RIGHT) {
        edges |= Qt::RightEdge;
    }
    if (zxdgEdges & ZXDG_TOPLEVEL_V6_RESIZE_EDGE_BOTTOM) {
        edges |= Qt::BottomEdge;
    }
    if (zxdgEdges & ZXDG_TOPLEVEL_V6_RESIZE_EDGE_LEFT) {
        edges |= Qt::LeftEdge;
    }

    emit q->resizeRequested(seat, edges, serial);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_max_size(Resource *resource,
                                                                  int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource->handle, -1, "width and height must be positive or zero");
        return;
    }
    next.maximumSize = QSize(width, height);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_min_size(Resource *resource,
                                                                  int32_t width, int32_t height)
{
    if (width < 0 || height < 0) {
        wl_resource_post_error(resource->handle, -1, "width and height must be positive or zero");
        return;
    }
    next.minimumSize = QSize(width, height);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_maximized(Resource *resource)
{
    Q_UNUSED(resource)
    emit q->maximizeRequested();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_unset_maximized(Resource *resource)
{
    Q_UNUSED(resource)
    emit q->unmaximizeRequested();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_fullscreen(Resource *resource,
                                                                    ::wl_resource *outputResource)
{
    Q_UNUSED(resource)
    OutputInterface *output = OutputInterface::get(outputResource);
    emit q->fullscreenRequested(output);
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_unset_fullscreen(Resource *resource)
{
    Q_UNUSED(resource)
    emit q->unfullscreenRequested();
}

void XdgToplevelV6InterfacePrivate::zxdg_toplevel_v6_set_minimized(Resource *resource)
{
    Q_UNUSED(resource)
    emit q->minimizeRequested();
}

XdgToplevelV6Interface::XdgToplevelV6Interface(XdgSurfaceV6Interface *surface, ::wl_resource *resource)
    : d(new XdgToplevelV6InterfacePrivate(this, surface))
{
    d->init(resource);
}

XdgToplevelV6Interface::~XdgToplevelV6Interface()
{
}

XdgShellV6Interface *XdgToplevelV6Interface::shell() const
{
    return d->xdgSurface->shell();
}

XdgSurfaceV6Interface *XdgToplevelV6Interface::xdgSurface() const
{
    return d->xdgSurface;
}

SurfaceInterface *XdgToplevelV6Interface::surface() const
{
    return d->xdgSurface->surface();
}

XdgToplevelV6Interface *XdgToplevelV6Interface::parentXdgToplevel() const
{
    return d->parentXdgToplevel;
}

QString XdgToplevelV6Interface::windowTitle() const
{
    return d->windowTitle;
}

QString XdgToplevelV6Interface::windowClass() const
{
    return d->windowClass;
}

QSize XdgToplevelV6Interface::minimumSize() const
{
    return d->current.minimumSize.isEmpty() ? QSize(0, 0) : d->current.minimumSize;
}

QSize XdgToplevelV6Interface::maximumSize() const
{
    return d->current.maximumSize.isEmpty() ? QSize(INT_MAX, INT_MAX) : d->current.maximumSize;
}

quint32 XdgToplevelV6Interface::sendConfigure(const QSize &size, const States &states)
{
    // Note that the states listed in the configure event must be an array of uint32_t.

    uint32_t statesData[4] = { 0 };
    int i = 0;

    if (states & State::MaximizedHorizontal && states & State::MaximizedVertical) {
        statesData[i++] = ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED;
    }
    if (states & State::FullScreen) {
        statesData[i++] = ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN;
    }
    if (states & State::Resizing) {
        statesData[i++] = ZXDG_TOPLEVEL_V6_STATE_RESIZING;
    }
    if (states & State::Activated) {
        statesData[i++] = ZXDG_TOPLEVEL_V6_STATE_ACTIVATED;
    }

    const QByteArray xdgStates = QByteArray::fromRawData(reinterpret_cast<char *>(statesData),
                                                         sizeof(uint32_t) * i);
    const quint32 serial = xdgSurface()->shell()->display()->nextSerial();

    d->send_configure(size.width(), size.height(), xdgStates);

    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface());
    xdgSurfacePrivate->send_configure(serial);
    xdgSurfacePrivate->isConfigured = true;

    return serial;
}

void XdgToplevelV6Interface::sendClose()
{
    d->send_close();
}

XdgToplevelV6Interface *XdgToplevelV6Interface::get(::wl_resource *resource)
{
    if (auto surface = QtWaylandServer::zxdg_toplevel_v6::Resource::fromResource(resource)) {
        return static_cast<XdgToplevelV6InterfacePrivate *>(surface->object())->q;
    }
    return nullptr;
}

XdgPopupV6InterfacePrivate::XdgPopupV6InterfacePrivate(XdgPopupV6Interface *popup,
                                                       XdgSurfaceV6Interface *surface)
    : SurfaceRole(surface->surface())
    , q(popup)
    , xdgSurface(surface)
{
}

void XdgPopupV6InterfacePrivate::commit()
{
    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface);

    if (xdgSurfacePrivate->isConfigured) {
        xdgSurfacePrivate->commit();
    } else {
        emit q->initializeRequested();
    }
}

void XdgPopupV6InterfacePrivate::zxdg_popup_v6_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete q;
}

void XdgPopupV6InterfacePrivate::zxdg_popup_v6_destroy(Resource *resource)
{
    // TODO: We need to post an error with the code ZXDG_SHELL_V6_ERROR_NOT_THE_TOPMOST_POPUP if
    // this popup is not the topmost grabbing popup. We most likely need a grab abstraction or
    // something to determine whether the given popup has an explicit grab.
    wl_resource_destroy(resource->handle);
}

void XdgPopupV6InterfacePrivate::zxdg_popup_v6_grab(Resource *resource, ::wl_resource *seatHandle, uint32_t serial)
{
    Q_UNUSED(resource)
    SeatInterface *seat = SeatInterface::get(seatHandle);
    emit q->grabRequested(seat, serial);
}

XdgPopupV6Interface::XdgPopupV6Interface(XdgSurfaceV6Interface *surface,
                                         XdgSurfaceV6Interface *parentSurface,
                                         const XdgPositionerV6 &positioner,
                                         ::wl_resource *resource)
    : d(new XdgPopupV6InterfacePrivate(this, surface))
{
    d->parentXdgSurface = parentSurface;
    d->positioner = positioner;
    d->init(resource);
}

XdgPopupV6Interface::~XdgPopupV6Interface()
{
}

XdgSurfaceV6Interface *XdgPopupV6Interface::parentXdgSurface() const
{
    return d->parentXdgSurface;
}

XdgSurfaceV6Interface *XdgPopupV6Interface::xdgSurface() const
{
    return d->xdgSurface;
}

SurfaceInterface *XdgPopupV6Interface::surface() const
{
    return d->xdgSurface->surface();
}

XdgPositionerV6 XdgPopupV6Interface::positioner() const
{
    return d->positioner;
}

quint32 XdgPopupV6Interface::sendConfigure(const QRect &rect)
{
    const quint32 serial = xdgSurface()->shell()->display()->nextSerial();

    d->send_configure(rect.x(), rect.y(), rect.width(), rect.height());

    auto xdgSurfacePrivate = XdgSurfaceV6InterfacePrivate::get(xdgSurface());
    xdgSurfacePrivate->send_configure(serial);
    xdgSurfacePrivate->isConfigured = true;

    return serial;
}

void XdgPopupV6Interface::sendPopupDone()
{
    d->send_popup_done();
}

XdgPopupV6Interface *XdgPopupV6Interface::get(::wl_resource *resource)
{
    if (auto popup = QtWaylandServer::zxdg_popup_v6::Resource::fromResource(resource)) {
        return static_cast<XdgPopupV6InterfacePrivate *>(popup->object())->q;
    }
    return nullptr;
}

XdgPositionerV6Private::XdgPositionerV6Private(::wl_resource *resource)
    : data(new XdgPositionerV6Data)
{
    init(resource);
}

XdgPositionerV6Private *XdgPositionerV6Private::get(wl_resource *resource)
{
    if (auto positioner = QtWaylandServer::zxdg_positioner_v6::Resource::fromResource(resource)) {
        return static_cast<XdgPositionerV6Private *>(positioner->object());
    }
    return nullptr;
}

void XdgPositionerV6Private::zxdg_positioner_v6_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void XdgPositionerV6Private::zxdg_positioner_v6_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_size(Resource *resource, int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "width and height must be positive and non-zero");
        return;
    }
    data->size = QSize(width, height);
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_anchor_rect(Resource *resource, int32_t x,
                                                                int32_t y, int32_t width, int32_t height)
{
    if (width < 1 || height < 1) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "width and height must be positive and non-zero");
        return;
    }
    data->anchorRect = QRect(x, y, width, height);
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_anchor(Resource *resource, uint32_t anchor)
{
    // In xdg-shell-unstable-v6, the anchor enum is a bitmask, which is not really a good API
    // choice because this means that one can set an anchor with opposite edges, which doesn't
    // make any sense. Luckily for us, this is not the case in xdg_wm_base!

    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_LEFT && anchor & ZXDG_POSITIONER_V6_ANCHOR_RIGHT) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "invalid combination of anchor edges (left and right)");
        return;
    }
    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_TOP && anchor & ZXDG_POSITIONER_V6_ANCHOR_BOTTOM) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "invalid combination of anchor edges (top and bottom)");
        return;
    }

    data->anchorEdges = Qt::Edges();

    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_LEFT) {
        data->anchorEdges |= Qt::LeftEdge;
    }
    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_TOP) {
        data->anchorEdges |= Qt::TopEdge;
    }
    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_RIGHT) {
        data->anchorEdges |= Qt::RightEdge;
    }
    if (anchor & ZXDG_POSITIONER_V6_ANCHOR_BOTTOM) {
        data->anchorEdges |= Qt::BottomEdge;
    }
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_gravity(Resource *resource, uint32_t gravity)
{
    // In xdg-shell-unstable-v6, the gravity enum is a bitmask, which is not really a good API
    // choice because this means that one can set opposite edges for the gravity, which doesn't
    // make any sense. Luckily for us, this is not the case in xdg_wm_base!

    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_LEFT && gravity & ZXDG_POSITIONER_V6_GRAVITY_RIGHT) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "invalid combination of gravity edges (left and right)");
        return;
    }
    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_TOP && gravity & ZXDG_POSITIONER_V6_GRAVITY_BOTTOM) {
        wl_resource_post_error(resource->handle, ZXDG_POSITIONER_V6_ERROR_INVALID_INPUT,
                               "invalid combination of gravity edges (top and bottom)");
        return;
    }

    data->gravityEdges = Qt::Edges();

    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_LEFT) {
        data->gravityEdges |= Qt::LeftEdge;
    }
    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_TOP) {
        data->gravityEdges |= Qt::TopEdge;
    }
    if (gravity & ZXDG_POSITIONER_V6_ANCHOR_RIGHT) {
        data->gravityEdges |= Qt::RightEdge;
    }
    if (gravity & ZXDG_POSITIONER_V6_GRAVITY_BOTTOM) {
        data->gravityEdges |= Qt::BottomEdge;
    }
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_constraint_adjustment(Resource *resource,
                                                                          uint32_t constraint_adjustment)
{
    Q_UNUSED(resource)

    if (constraint_adjustment & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_FLIP_X) {
        data->flipConstraintAdjustments |= Qt::Horizontal;
    } else {
        data->flipConstraintAdjustments &= ~Qt::Horizontal;
    }

    if (constraint_adjustment & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_FLIP_Y) {
        data->flipConstraintAdjustments |= Qt::Vertical;
    } else {
        data->flipConstraintAdjustments &= ~Qt::Vertical;
    }

    if (constraint_adjustment & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_SLIDE_X) {
        data->slideConstraintAdjustments |= Qt::Horizontal;
    } else {
        data->slideConstraintAdjustments &= ~Qt::Horizontal;
    }

    if (constraint_adjustment & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_SLIDE_Y) {
        data->slideConstraintAdjustments |= Qt::Vertical;
    } else {
        data->slideConstraintAdjustments &= ~Qt::Vertical;
    }

    if (constraint_adjustment & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_RESIZE_X) {
        data->resizeConstraintAdjustments |= Qt::Horizontal;
    } else {
        data->resizeConstraintAdjustments &= ~Qt::Horizontal;
    }

    if (constraint_adjustment & ZXDG_POSITIONER_V6_CONSTRAINT_ADJUSTMENT_RESIZE_Y) {
        data->resizeConstraintAdjustments |= Qt::Vertical;
    } else {
        data->resizeConstraintAdjustments &= ~Qt::Vertical;
    }
}

void XdgPositionerV6Private::zxdg_positioner_v6_set_offset(Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource)
    data->offset = QPoint(x, y);
}

XdgPositionerV6::XdgPositionerV6()
    : d(new XdgPositionerV6Data)
{
}

XdgPositionerV6::XdgPositionerV6(const XdgPositionerV6 &other)
    : d(other.d)
{
}

XdgPositionerV6::~XdgPositionerV6()
{
}

XdgPositionerV6 &XdgPositionerV6::operator=(const XdgPositionerV6 &other)
{
    d = other.d;
    return *this;
}

bool XdgPositionerV6::isComplete() const
{
    return d->size.isValid() && d->anchorRect.isValid();
}

Qt::Orientations XdgPositionerV6::slideConstraintAdjustments() const
{
    return d->slideConstraintAdjustments;
}

Qt::Orientations XdgPositionerV6::flipConstraintAdjustments() const
{
    return d->flipConstraintAdjustments;
}

Qt::Orientations XdgPositionerV6::resizeConstraintAdjustments() const
{
    return d->resizeConstraintAdjustments;
}

Qt::Edges XdgPositionerV6::anchorEdges() const
{
    return d->anchorEdges;
}

Qt::Edges XdgPositionerV6::gravityEdges() const
{
    return d->gravityEdges;
}

QSize XdgPositionerV6::size() const
{
    return d->size;
}

QRect XdgPositionerV6::anchorRect() const
{
    return d->anchorRect;
}

QPoint XdgPositionerV6::offset() const
{
    return d->offset;
}

XdgPositionerV6 XdgPositionerV6::get(::wl_resource *resource)
{
    XdgPositionerV6Private *xdgPositionerPrivate = XdgPositionerV6Private::get(resource);
    if (xdgPositionerPrivate) {
        return XdgPositionerV6(xdgPositionerPrivate->data);
    }
    return XdgPositionerV6();
}

XdgPositionerV6::XdgPositionerV6(const QSharedDataPointer<XdgPositionerV6Data> &data)
    : d(data)
{
}

} // namespace KWin
