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

#include "openglplatformtexture_dmabuf.h"

#include "scene.h"

#include <KWayland/Server/buffer_interface.h>

namespace KWin
{

OpenGLDmaBufPlatformTexturePrivate::OpenGLDmaBufPlatformTexturePrivate(OpenGLPlatformTexture *texture)
    : OpenGLPlatformTexturePrivate(texture)
{
}

OpenGLDmaBufPlatformTexturePrivate::~OpenGLDmaBufPlatformTexturePrivate()
{
}

bool OpenGLDmaBufPlatformTexturePrivate::isCompatible(const WaylandPlatformSurface *platformSurface) const
{
    return platformSurface->buffer()->linuxDmabufBuffer();
}

bool OpenGLDmaBufPlatformTexturePrivate::create(WaylandPlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLDmaBufPlatformTexturePrivate::update(WaylandPlatformSurface *platformSurface, const QRegion &damage)
{
    Q_UNUSED(damage)
    return false;
}

} // namespace KWin
