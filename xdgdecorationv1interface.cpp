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

#include "xdgdecorationv1interface.h"
#include "xdgdecorationv1interface_p.h"
#include "xdgshellinterface.h"

#include <KWayland/Server/display.h>

using namespace KWayland::Server;

namespace KWin
{

// TODO: We need to wait for an ack_configure either here or in xdgshellclient.cpp.

XdgDecorationManagerV1InterfacePrivate::XdgDecorationManagerV1InterfacePrivate(XdgDecorationManagerV1Interface *manager)
    : q(manager)
{
}

void XdgDecorationManagerV1InterfacePrivate::registerXdgDecoration(XdgToplevelDecorationV1Interface *decoration)
{
    decorations.insert(decoration->toplevel(), decoration);
}

void XdgDecorationManagerV1InterfacePrivate::unregisterXdgDecoration(XdgToplevelDecorationV1Interface *decoration)
{
    decorations.remove(decoration->toplevel());
}

XdgDecorationManagerV1InterfacePrivate *XdgDecorationManagerV1InterfacePrivate::get(XdgDecorationManagerV1Interface *manager)
{
    return manager->d.data();
}

void XdgDecorationManagerV1InterfacePrivate::zxdg_decoration_manager_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgDecorationManagerV1InterfacePrivate::zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource,
                                                                                                uint32_t id,
                                                                                                ::wl_resource *toplevelResource)
{
    XdgToplevelInterface *toplevel = XdgToplevelInterface::get(toplevelResource);
    if (!toplevel) {
        wl_resource_post_error(resource->handle, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED,
                               "no xdg-toplevel object");
        return;
    }

    if (decorations.contains(toplevel)) {
        wl_resource_post_error(resource->handle, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ALREADY_CONSTRUCTED,
                               "decoration has been already constructed");
    }

    wl_resource *decorationResource = wl_resource_create(resource->client(),
                                                         &zxdg_toplevel_decoration_v1_interface,
                                                         wl_resource_get_version(resource->handle),
                                                         id);

    auto decoration = new XdgToplevelDecorationV1Interface(q, toplevel, decorationResource);
    registerXdgDecoration(decoration);

    emit q->decorationCreated(decoration);
}

XdgDecorationManagerV1Interface::XdgDecorationManagerV1Interface(Display *display, QObject *parent)
    : QObject(parent)
    , d(new XdgDecorationManagerV1InterfacePrivate(this))
{
    d->init(*display, 1);
}

XdgDecorationManagerV1Interface::~XdgDecorationManagerV1Interface()
{
}

XdgToplevelDecorationV1InterfacePrivate::XdgToplevelDecorationV1InterfacePrivate(XdgToplevelDecorationV1Interface *decoration)
    : q(decoration)
{
}

void XdgToplevelDecorationV1InterfacePrivate::zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    XdgDecorationManagerV1InterfacePrivate::get(manager)->unregisterXdgDecoration(q);
    delete q;
}

void XdgToplevelDecorationV1InterfacePrivate::zxdg_toplevel_decoration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgToplevelDecorationV1InterfacePrivate::zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode)
{
    Q_UNUSED(resource)

    switch (mode) {
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
        preferredMode = XdgToplevelDecorationV1Interface::Mode::Client;
        break;
    case ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
        preferredMode = XdgToplevelDecorationV1Interface::Mode::Server;
        break;
    default:
        preferredMode = XdgToplevelDecorationV1Interface::Mode::Unspecified;
        break;
    }

    emit q->preferredModeChanged(preferredMode);
}

void XdgToplevelDecorationV1InterfacePrivate::zxdg_toplevel_decoration_v1_unset_mode(Resource *resource)
{
    Q_UNUSED(resource)
    preferredMode = XdgToplevelDecorationV1Interface::Mode::Unspecified;
    emit q->preferredModeChanged(preferredMode);
}

XdgToplevelDecorationV1Interface::XdgToplevelDecorationV1Interface(XdgDecorationManagerV1Interface *manager,
                                                                   XdgToplevelInterface *toplevel,
                                                                   ::wl_resource *resource)
    : d(new XdgToplevelDecorationV1InterfacePrivate(this))
{
    d->manager = manager;
    d->toplevel = toplevel;
    d->init(resource);
}

XdgToplevelDecorationV1Interface::~XdgToplevelDecorationV1Interface()
{
}

XdgDecorationManagerV1Interface *XdgToplevelDecorationV1Interface::manager() const
{
    return d->manager;
}

XdgToplevelInterface *XdgToplevelDecorationV1Interface::toplevel() const
{
    return d->toplevel;
}

XdgToplevelDecorationV1Interface::Mode XdgToplevelDecorationV1Interface::preferredMode() const
{
    return d->preferredMode;
}

void XdgToplevelDecorationV1Interface::sendConfigure(Mode mode)
{
    switch (mode) {
    case Mode::Client:
        d->send_configure(ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
        break;
    case Mode::Server:
        d->send_configure(ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
        break;
    case Mode::Unspecified:
        break;
    }
}

} // namespace KWin
