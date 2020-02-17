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

#include "xdgdecorationv1interface.h"

#include "qwayland-server-xdg-decoration-unstable-v1.h"

namespace KWin
{

class XdgDecorationManagerV1InterfacePrivate : public QtWaylandServer::zxdg_decoration_manager_v1
{
public:
    XdgDecorationManagerV1InterfacePrivate(XdgDecorationManagerV1Interface *manager);

    void registerXdgDecoration(XdgToplevelDecorationV1Interface *decoration);
    void unregisterXdgDecoration(XdgToplevelDecorationV1Interface *decoration);

    static XdgDecorationManagerV1InterfacePrivate *get(XdgDecorationManagerV1Interface *manager);

    XdgDecorationManagerV1Interface *q;
    QMap<XdgToplevelInterface *, XdgToplevelDecorationV1Interface *> decorations;

protected:
    void zxdg_decoration_manager_v1_destroy(Resource *resource) override;
    void zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource, uint32_t id, ::wl_resource *toplevel) override;
};

class XdgToplevelDecorationV1InterfacePrivate : public QtWaylandServer::zxdg_toplevel_decoration_v1
{
public:
    XdgToplevelDecorationV1InterfacePrivate(XdgToplevelDecorationV1Interface *decoration);

    XdgToplevelDecorationV1Interface *q;
    XdgDecorationManagerV1Interface *manager;
    XdgToplevelInterface *toplevel;
    XdgToplevelDecorationV1Interface::Mode preferredMode = XdgToplevelDecorationV1Interface::Mode::Unspecified;

protected:
    void zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_destroy(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode) override;
    void zxdg_toplevel_decoration_v1_unset_mode(Resource *resource) override;
};

} // namespace KWin
