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

#include "buffer.h"

namespace KWin
{

Buffer::Buffer(QObject *parent)
    : QObject(parent)
{
}

Buffer::~Buffer()
{
}

void Buffer::setToplevel(Toplevel *toplevel)
{
    Q_UNUSED(toplevel)
}

Toplevel *Buffer::toplevel() const
{
    return nullptr;
}

bool Buffer::isDirty() const
{
    return false;
}

bool Buffer::isValid() const
{
    return false;
}

bool Buffer::hasAlphaChannel() const
{
    return false;
}

QRegion Buffer::damage() const
{
    return QRegion();
}

QRegion Buffer::shape() const
{
    return QRegion();
}

qreal Buffer::scale() const
{
    return 1;
}

bool Buffer::create()
{
    return false;
}

void Buffer::update()
{
}

GLTexture *Buffer::toOpenGLTexture() const
{
    return nullptr;
}

QImage Buffer::toImage() const
{
    return QImage();
}

} // namespace KWin
