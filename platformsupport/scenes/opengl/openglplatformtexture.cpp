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

#include "openglplatformtexture.h"
#include "openglplatformtexture_p.h"

namespace KWin
{

OpenGLPlatformTexture::OpenGLPlatformTexture(OpenGLBackend *backend)
    : m_backend(backend)
{
}

OpenGLPlatformTexture::~OpenGLPlatformTexture()
{
}

OpenGLBackend *OpenGLPlatformTexture::backend() const
{
    return m_backend;
}

OpenGLPlatformTexturePrivate::OpenGLPlatformTexturePrivate(OpenGLPlatformTexture *texture)
    : q(texture)
{
}

OpenGLPlatformTexturePrivate::~OpenGLPlatformTexturePrivate()
{
}

OpenGLBackend *OpenGLPlatformTexturePrivate::backend() const
{
    return q->backend();
}

bool OpenGLPlatformTexture::create(InternalPlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLPlatformTexture::create(WaylandPlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLPlatformTexture::create(X11PlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLPlatformTexture::update(InternalPlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLPlatformTexture::update(WaylandPlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLPlatformTexture::update(X11PlatformSurface *platformSurface)
{
    return false;
}

bool OpenGLPlatformTexturePrivate::isCompatible(const InternalPlatformSurface *platformSurface) const
{
    Q_UNUSED(platformSurface)
    return false;
}

bool OpenGLPlatformTexturePrivate::isCompatible(const WaylandPlatformSurface *platformSurface) const
{
    Q_UNUSED(platformSurface)
    return false;
}

bool OpenGLPlatformTexturePrivate::isCompatible(const X11PlatformSurface *platformSurface) const
{
    Q_UNUSED(platformSurface)
    return false;
}

bool OpenGLPlatformTexturePrivate::create(InternalPlatformSurface *platformsurface)
{
    Q_UNUSED(platformsurface)
    return false;
}

bool OpenGLPlatformTexturePrivate::create(WaylandPlatformSurface *platformSurface)
{
    Q_UNUSED(platformSurface)
    return false;
}

bool OpenGLPlatformTexturePrivate::create(X11PlatformSurface *platformSurface)
{
    Q_UNUSED(platformSurface)
    return false;
}

bool OpenGLPlatformTexturePrivate::update(InternalPlatformSurface *platformSurface, const QRegion &damage)
{
    Q_UNUSED(platformSurface)
    Q_UNUSED(damage)
    return false;
}

bool OpenGLPlatformTexturePrivate::update(WaylandPlatformSurface *platformSurface, const QRegion &damage)
{
    Q_UNUSED(platformSurface)
    Q_UNUSED(damage)
    return false;
}

bool OpenGLPlatformTexturePrivate::update(X11PlatformSurface *platformSurface, const QRegion &damage)
{
    Q_UNUSED(platformSurface)
    Q_UNUSED(damage)
    return false;
}

} // namespace KWin
