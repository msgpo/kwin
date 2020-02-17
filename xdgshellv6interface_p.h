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

#include "xdgshellv6interface.h"
#include "qwayland-server-xdg-shell-unstable-v6.h"

#include <KWayland/Server/surface_interface.h>
#include <KWayland/Server/surfacerole.h>

using namespace KWayland::Server;

namespace KWin
{

class XdgShellV6InterfacePrivate : public QtWaylandServer::zxdg_shell_v6
{
public:
    XdgShellV6InterfacePrivate(XdgShellV6Interface *shell);

    void registerXdgSurface(XdgSurfaceV6Interface *surface);
    void unregisterXdgSurface(XdgSurfaceV6Interface *surface);

    void registerPing(quint32 serial);

    static XdgShellV6InterfacePrivate *get(XdgShellV6Interface *shell);

    XdgShellV6Interface *q;
    Display *display;
    QMap<quint32, QTimer *> pings;

protected:
    void zxdg_shell_v6_destroy(Resource *resource) override;
    void zxdg_shell_v6_create_positioner(Resource *resource, uint32_t id) override;
    void zxdg_shell_v6_get_xdg_surface(Resource *resource, uint32_t id, ::wl_resource *surface) override;
    void zxdg_shell_v6_pong(Resource *resource, uint32_t serial) override;

private:
    QMultiMap<wl_client *, XdgSurfaceV6Interface *> xdgSurfaces;
};

class XdgPositionerV6Data : public QSharedData
{
public:
    Qt::Orientations flipConstraintAdjustments;
    Qt::Orientations slideConstraintAdjustments;
    Qt::Orientations resizeConstraintAdjustments;
    Qt::Edges anchorEdges;
    Qt::Edges gravityEdges;
    QPoint offset;
    QSize size;
    QRect anchorRect;
};

class XdgPositionerV6Private : public QtWaylandServer::zxdg_positioner_v6
{
public:
    XdgPositionerV6Private(::wl_resource *resource);

    QSharedDataPointer<XdgPositionerV6Data> data;

    static XdgPositionerV6Private *get(::wl_resource *resource);

protected:
    void zxdg_positioner_v6_destroy_resource(Resource *resource) override;
    void zxdg_positioner_v6_destroy(Resource *resource) override;
    void zxdg_positioner_v6_set_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_positioner_v6_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zxdg_positioner_v6_set_anchor(Resource *resource, uint32_t anchor) override;
    void zxdg_positioner_v6_set_gravity(Resource *resource, uint32_t gravity) override;
    void zxdg_positioner_v6_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void zxdg_positioner_v6_set_offset(Resource *resource, int32_t x, int32_t y) override;
};

class XdgSurfaceV6InterfacePrivate : public QtWaylandServer::zxdg_surface_v6
{
public:
    XdgSurfaceV6InterfacePrivate(XdgSurfaceV6Interface *xdgSurface);

    void commit();

    XdgSurfaceV6Interface *q;
    XdgShellV6Interface *shell;
    QPointer<XdgToplevelV6Interface> toplevel;
    QPointer<XdgPopupV6Interface> popup;
    QPointer<SurfaceInterface> surface;
    bool isConfigured = false;

    struct State
    {
        QRect windowGeometry;
    };

    State next;
    State current;

    static XdgSurfaceV6InterfacePrivate *get(XdgSurfaceV6Interface *surface);

protected:
    void zxdg_surface_v6_destroy_resource(Resource *resource) override;
    void zxdg_surface_v6_destroy(Resource *resource) override;
    void zxdg_surface_v6_get_toplevel(Resource *resource, uint32_t id) override;
    void zxdg_surface_v6_get_popup(Resource *resource, uint32_t id, ::wl_resource *parent, ::wl_resource *positioner) override;
    void zxdg_surface_v6_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zxdg_surface_v6_ack_configure(Resource *resource, uint32_t serial) override;
};

class XdgToplevelV6InterfacePrivate : public SurfaceRole, public QtWaylandServer::zxdg_toplevel_v6
{
public:
    XdgToplevelV6InterfacePrivate(XdgToplevelV6Interface *toplevel, XdgSurfaceV6Interface *surface);

    void commit() override;

    XdgToplevelV6Interface *q;
    XdgToplevelV6Interface *parentXdgToplevel = nullptr;
    XdgSurfaceV6Interface *xdgSurface;

    QString windowTitle;
    QString windowClass;

    struct State
    {
        QSize minimumSize;
        QSize maximumSize;
    };

    State next;
    State current;

protected:
    void zxdg_toplevel_v6_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_v6_destroy(Resource *resource) override;
    void zxdg_toplevel_v6_set_parent(Resource *resource, ::wl_resource *parent) override;
    void zxdg_toplevel_v6_set_title(Resource *resource, const QString &title) override;
    void zxdg_toplevel_v6_set_app_id(Resource *resource, const QString &app_id) override;
    void zxdg_toplevel_v6_show_window_menu(Resource *resource, ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void zxdg_toplevel_v6_move(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
    void zxdg_toplevel_v6_resize(Resource *resource, ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void zxdg_toplevel_v6_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_toplevel_v6_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void zxdg_toplevel_v6_set_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_unset_maximized(Resource *resource) override;
    void zxdg_toplevel_v6_set_fullscreen(Resource *resource, ::wl_resource *output) override;
    void zxdg_toplevel_v6_unset_fullscreen(Resource *resource) override;
    void zxdg_toplevel_v6_set_minimized(Resource *resource) override;
};

class XdgPopupV6InterfacePrivate : public SurfaceRole, public QtWaylandServer::zxdg_popup_v6
{
public:
    XdgPopupV6InterfacePrivate(XdgPopupV6Interface *popup, XdgSurfaceV6Interface *surface);

    void commit() override;

    XdgPopupV6Interface *q;
    QPointer<XdgSurfaceV6Interface> parentXdgSurface;
    XdgSurfaceV6Interface *xdgSurface;
    XdgPositionerV6 positioner;

protected:
    void zxdg_popup_v6_destroy_resource(Resource *resource) override;
    void zxdg_popup_v6_destroy(Resource *resource) override;
    void zxdg_popup_v6_grab(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
};

} // namespace KWin
