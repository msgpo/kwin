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

#include "buffer_wayland.h"
#include "buffer_wayland_p.h"

#include "composite.h"
#include "scene.h"

#include <KWaylandServer/buffer_interface.h>
#include <KWaylandServer/surface_interface.h>

using namespace KWaylandServer;

namespace KWin
{

BufferWayland::BufferWayland(QObject *parent)
    : Buffer(parent)
    , d(compositor()->scene()->createBufferWaylandPrivate())
{
}

BufferWayland::~BufferWayland()
{
    if (d->buffer) {
        disconnect(d->buffer, &BufferInterface::aboutToBeDestroyed, d->buffer, &BufferInterface::unref);
        d->buffer->unref();
    }
}

bool BufferWayland::isDirty() const
{
    return false;
}

bool BufferWayland::isValid() const
{
    return d->buffer;
}

bool BufferWayland::create()
{
    if (d->surface)
        d->surface->resetTrackedDamage();

    return d->create();
}

void BufferWayland::update()
{
    BufferInterface *buffer = d->surface->buffer();
    if (d->buffer == buffer)
        return;

    if (d->buffer) {
        disconnect(d->buffer, &BufferInterface::aboutToBeDestroyed, d->buffer, &BufferInterface::unref);
        d->buffer->unref();
    }

    d->buffer = buffer;

    if (d->buffer) {
        connect(d->buffer, &BufferInterface::aboutToBeDestroyed, d->buffer, &BufferInterface::unref);
        d->buffer->ref();
    }

    d->update();

    if (d->surface)
        d->surface->resetTrackedDamage();
}

void BufferWayland::setToplevel(Toplevel *toplevel)
{
    d->toplevel = toplevel;
}

Toplevel *BufferWayland::toplevel() const
{
    return d->toplevel;
}

GLTexture *BufferWayland::toOpenGLTexture() const
{
    return d->texture;
}

QImage BufferWayland::toImage() const
{
    return d->image;
}

} // namespace KWin
