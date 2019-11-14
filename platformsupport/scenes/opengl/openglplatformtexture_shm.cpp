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

#include "openglplatformtexture_shm.h"

#include "scene.h"

#include <KWayland/Server/buffer_interface.h>

namespace KWin
{

OpenGLShmPlatformTexturePrivate::OpenGLShmPlatformTexturePrivate(OpenGLPlatformTexture *texture)
    : OpenGLPlatformTexturePrivate(texture)
{
}

OpenGLShmPlatformTexturePrivate::~OpenGLShmPlatformTexturePrivate()
{
}

bool OpenGLShmPlatformTexturePrivate::isCompatible(const WaylandPlatformSurface *platformSurface) const
{
    return platformSurface->buffer()->shmBuffer();
}

bool OpenGLShmPlatformTexturePrivate::create(WaylandPlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLShmPlatformTexturePrivate::update(WaylandPlatformSurface *platformSurface, const QRegion &damage)
{
    return false;
}

} // namespace KWin
