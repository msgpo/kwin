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

#include "xdgshellinterface.h"
#include "qwayland-server-xdg-shell.h"

#include <KWayland/Server/surface_interface.h>
#include <KWayland/Server/surfacerole.h>

using namespace KWayland::Server;

namespace KWin
{

class XdgShellInterfacePrivate : public QtWaylandServer::xdg_wm_base
{
public:
    XdgShellInterfacePrivate(XdgShellInterface *shell);

    void registerXdgSurface(XdgSurfaceInterface *surface);
    void unregisterXdgSurface(XdgSurfaceInterface *surface);

    void registerPing(quint32 serial);

    static XdgShellInterfacePrivate *get(XdgShellInterface *shell);

    XdgShellInterface *q;
    Display *display;
    QMap<quint32, QTimer *> pings;

protected:
    void xdg_wm_base_destroy(Resource *resource) override;
    void xdg_wm_base_create_positioner(Resource *resource, uint32_t id) override;
    void xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id, ::wl_resource *surface) override;
    void xdg_wm_base_pong(Resource *resource, uint32_t serial) override;

private:
    QMultiMap<wl_client *, XdgSurfaceInterface *> xdgSurfaces;
};

class XdgPositionerData : public QSharedData
{
public:
    Qt::Orientations slideConstraintAdjustments;
    Qt::Orientations flipConstraintAdjustments;
    Qt::Orientations resizeConstraintAdjustments;
    Qt::Edges anchorEdges;
    Qt::Edges gravityEdges;
    QPoint offset;
    QSize size;
    QRect anchorRect;
};

class XdgPositionerPrivate : public QtWaylandServer::xdg_positioner
{
public:
    XdgPositionerPrivate(::wl_resource *resource);

    QSharedDataPointer<XdgPositionerData> data;

    static XdgPositionerPrivate *get(::wl_resource *resource);

protected:
    void xdg_positioner_destroy_resource(Resource *resource) override;
    void xdg_positioner_destroy(Resource *resource) override;
    void xdg_positioner_set_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor(Resource *resource, uint32_t anchor) override;
    void xdg_positioner_set_gravity(Resource *resource, uint32_t gravity) override;
    void xdg_positioner_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void xdg_positioner_set_offset(Resource *resource, int32_t x, int32_t y) override;
};

class XdgSurfaceInterfacePrivate : public QtWaylandServer::xdg_surface
{
public:
    XdgSurfaceInterfacePrivate(XdgSurfaceInterface *xdgSurface);

    void commit();

    XdgSurfaceInterface *q;
    XdgShellInterface *shell;
    QPointer<XdgToplevelInterface> toplevel;
    QPointer<XdgPopupInterface> popup;
    QPointer<SurfaceInterface> surface;
    bool isConfigured = false;

    struct State
    {
        QRect windowGeometry;
    };

    State next;
    State current;

    static XdgSurfaceInterfacePrivate *get(XdgSurfaceInterface *surface);

protected:
    void xdg_surface_destroy_resource(Resource *resource) override;
    void xdg_surface_destroy(Resource *resource) override;
    void xdg_surface_get_toplevel(Resource *resource, uint32_t id) override;
    void xdg_surface_get_popup(Resource *resource, uint32_t id, ::wl_resource *parent, ::wl_resource *positioner) override;
    void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void xdg_surface_ack_configure(Resource *resource, uint32_t serial) override;
};

class XdgToplevelInterfacePrivate : public SurfaceRole, public QtWaylandServer::xdg_toplevel
{
public:
    XdgToplevelInterfacePrivate(XdgToplevelInterface *toplevel, XdgSurfaceInterface *surface);

    void commit() override;

    XdgToplevelInterface *q;
    XdgToplevelInterface *parentXdgToplevel = nullptr;
    XdgSurfaceInterface *xdgSurface;

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
    void xdg_toplevel_destroy_resource(Resource *resource) override;
    void xdg_toplevel_destroy(Resource *resource) override;
    void xdg_toplevel_set_parent(Resource *resource, ::wl_resource *parent) override;
    void xdg_toplevel_set_title(Resource *resource, const QString &title) override;
    void xdg_toplevel_set_app_id(Resource *resource, const QString &app_id) override;
    void xdg_toplevel_show_window_menu(Resource *resource, ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void xdg_toplevel_move(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
    void xdg_toplevel_resize(Resource *resource, ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void xdg_toplevel_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_maximized(Resource *resource) override;
    void xdg_toplevel_unset_maximized(Resource *resource) override;
    void xdg_toplevel_set_fullscreen(Resource *resource, ::wl_resource *output) override;
    void xdg_toplevel_unset_fullscreen(Resource *resource) override;
    void xdg_toplevel_set_minimized(Resource *resource) override;
};

class XdgPopupInterfacePrivate : public SurfaceRole, public QtWaylandServer::xdg_popup
{
public:
    XdgPopupInterfacePrivate(XdgPopupInterface *popup, XdgSurfaceInterface *surface);

    void commit() override;

    XdgPopupInterface *q;
    XdgSurfaceInterface *parentXdgSurface;
    XdgSurfaceInterface *xdgSurface;
    XdgPositioner positioner;

protected:
    void xdg_popup_destroy_resource(Resource *resource) override;
    void xdg_popup_destroy(Resource *resource) override;
    void xdg_popup_grab(Resource *resource, ::wl_resource *seat, uint32_t serial) override;
};

} // namespace KWin
