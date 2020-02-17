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

struct wl_resource;

namespace KWayland
{
namespace Server
{
class Display;
}
}

namespace KWin
{

class XdgDecorationManagerV1InterfacePrivate;
class XdgToplevelDecorationV1Interface;
class XdgToplevelDecorationV1InterfacePrivate;
class XdgToplevelInterface;

class XdgDecorationManagerV1Interface : public QObject
{
    Q_OBJECT

public:
    XdgDecorationManagerV1Interface(KWayland::Server::Display *display, QObject *parent = nullptr);
    /**
     * Destructs the XdgDecorationManagerV1Interface object.
     */
    ~XdgDecorationManagerV1Interface() override;

Q_SIGNALS:
    void decorationCreated(XdgToplevelDecorationV1Interface *decoration);

private:
    QScopedPointer<XdgDecorationManagerV1InterfacePrivate> d;
    friend class XdgDecorationManagerV1InterfacePrivate;
};

class XdgToplevelDecorationV1Interface : public QObject
{
    Q_OBJECT

public:
    enum class Mode {
        Unspecified,
        Client,
        Server,
    };
    Q_ENUM(Mode)

    /**
     * Constructs a XdgToplevelDecorationV1Interface for the given xdg-toplevel @p toplevel and
     * initializes it with @p manager and @p resource.
     */
    XdgToplevelDecorationV1Interface(XdgDecorationManagerV1Interface *manager,
                                     XdgToplevelInterface *toplevel, ::wl_resource *resource);
    /**
     * Destructs the XdgToplevelDecorationV1Interface object.
     */
    ~XdgToplevelDecorationV1Interface() override;

    /**
     * Returns the decoration manager for this XdgToplevelDecorationV1Interface.
     */
    XdgDecorationManagerV1Interface *manager() const;
    /**
     * Returns the toplevel for this XdgToplevelDecorationV1Interface.
     */
    XdgToplevelInterface *toplevel() const;
    /**
     * Returns the decoration mode preferred by the client.
     */
    Mode preferredMode() const;
    /**
     * Sends a configure event to the client. @p mode indicates the decoration mode the client
     * should be using. The client must send an ack_configure in response to this event.
     *
     * @see XdgToplevelInterface::sendConfigure
     */
    void sendConfigure(Mode mode);

Q_SIGNALS:
    /**
     * This signal is emitted when the client has specified the preferred decoration mode. The
     * compositor can decide not to use the client's mode and enforce a different mode instead.
     */
    void preferredModeChanged(Mode mode);

private:
    QScopedPointer<XdgToplevelDecorationV1InterfacePrivate> d;
};

} // namespace KWin
