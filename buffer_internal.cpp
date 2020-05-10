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

#include "buffer_internal.h"
#include "buffer_internal_p.h"

#include "composite.h"
#include "scene.h"

namespace KWin
{

BufferInternal::BufferInternal(QObject *parent)
    : Buffer(parent)
    , d(compositor()->scene()->createBufferInternalPrivate())
{
}

BufferInternal::~BufferInternal()
{
}

bool BufferInternal::isDirty() const
{
    return false;
}

bool BufferInternal::isValid() const
{
    return d->texture || !d->image.isNull();
}

bool BufferInternal::create()
{
    return d->create();
}

void BufferInternal::update()
{
    d->update();
}

void BufferInternal::setToplevel(Toplevel *toplevel)
{
    d->toplevel = toplevel;
}

Toplevel *BufferInternal::toplevel() const
{
    return d->toplevel;
}

GLTexture *BufferInternal::toOpenGLTexture() const
{
    return d->texture;
}

QImage BufferInternal::toImage() const
{
    return d->image;
}

} // namespace KWin
