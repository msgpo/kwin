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

#include "buffer_x11.h"
#include "buffer_x11_p.h"

#include "composite.h"
#include "scene.h"

namespace KWin
{

BufferX11::BufferX11(QObject *parent)
    : Buffer(parent)
    , d(compositor()->scene()->createBufferX11Private())
{
}

BufferX11::~BufferX11()
{
    if (d->pixmap != XCB_PIXMAP_NONE)
        xcb_free_pixmap(connection(), d->pixmap);
}

void BufferX11::setToplevel(Toplevel *toplevel)
{
    d->toplevel = toplevel;
}

Toplevel *BufferX11::toplevel() const
{
    return d->toplevel;
}

bool BufferX11::isDirty() const
{
    return false;
}

bool BufferX11::isValid() const
{
    return d->pixmap != XCB_PIXMAP_NONE;
}

bool BufferX11::create()
{
    XServerGrabber grabber;

    const xcb_pixmap_t pixmap = xcb_generate_id(connection());
    const xcb_void_cookie_t cookie =
            xcb_composite_name_window_pixmap_checked(connection(), toplevel()->frameId(), pixmap);
    Xcb::WindowAttributes windowAttributes(toplevel()->frameId());
    Xcb::WindowGeometry windowGeometry(toplevel()->frameId());
    if (xcb_generic_error_t *error = xcb_request_check(connection(), cookie)) {
        qCDebug(KWIN_CORE) << "Creating window pixmap failed:" << error->error_code;
        free(error);
        return false;
    }

    if (!windowAttributes || windowAttributes->map_state != XCB_MAP_STATE_VIEWABLE) {
        qCDebug(KWIN_CORE) << "Creating window pixmap failed:" << this;
        xcb_free_pixmap(connection(), pixmap);
        return false;
    }

    const QRect bufferGeometry = toplevel()->bufferGeometry();
    if (windowGeometry.size() != bufferGeometry.size()) {
        qCDebug(KWIN_CORE) << "Creating window pixmap failed:" << this;
        xcb_free_pixmap(connection(), pixmap);
        return false;
    }

    d->pixmap = pixmap;

    return d->create();
}

void BufferX11::update()
{
    d->update();
}

} // namespace KWin
