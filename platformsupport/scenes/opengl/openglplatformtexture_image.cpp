/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2019 Vlad Zahorodnii <vladzzag@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "openglplatformtexture_image.h"
#include "openglplatformtexture.h"

#include "scene.h"

namespace KWin
{

OpenGLImagePlatformTexturePrivate::OpenGLImagePlatformTexturePrivate(OpenGLPlatformTexture *texture)
    : OpenGLPlatformTexturePrivate(texture)
{
}

OpenGLImagePlatformTexturePrivate::~OpenGLImagePlatformTexturePrivate()
{
}

bool OpenGLImagePlatformTexturePrivate::isCompatible(const InternalPlatformSurface *platformSurface) const
{
    return !platformSurface->image().isNull();
}

bool OpenGLImagePlatformTexturePrivate::create(InternalPlatformSurface *platformSurface)
{
    const QImage image = platformSurface->image();

    q->setData(image);
    q->setWrapMode(GL_CLAMP_TO_EDGE);
    q->setFilter(GL_NEAREST);

    return true;
}

bool OpenGLImagePlatformTexturePrivate::update(InternalPlatformSurface *platformSurface, const QRegion &damage)
{
    const QImage buffer = platformSurface->image();
    const qreal devicePixelRatio = buffer.devicePixelRatio();

    q->bind();

    for (const QRect &rect : damage) {
        const QPoint position = rect.topLeft() * devicePixelRatio;
        const QSize size = rect.size() * devicePixelRatio;
        q->update(buffer, position, QRect(position, size));
    }

    q->unbind();

    return true;
}

} // namespace KWin
