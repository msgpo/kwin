/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009, 2010, 2011 Martin Gräßlin <mgraesslin@kde.org>

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
#include "texture.h"
#include "backend.h"
#include "scene.h"

namespace KWin
{

SceneOpenGLTexture::SceneOpenGLTexture(OpenGLBackend *backend)
    : GLTexture(*backend->createBackendTexture(this))
{
}

SceneOpenGLTexture::~SceneOpenGLTexture()
{
}

bool SceneOpenGLTexture::loadTexture(InternalPlatformSurface *platformSurface)
{
    return false;
}

bool SceneOpenGLTexture::loadTexture(WaylandPlatformSurface *platformSurface)
{
    return false;
}

bool SceneOpenGLTexture::loadTexture(X11PlatformSurface *platformSurface)
{
    return false;
}

void SceneOpenGLTexture::updateTexture(InternalPlatformSurface *platformSurface)
{
    Q_D(SceneOpenGLTexture);
    d->update(platformSurface);
}

void SceneOpenGLTexture::updateTexture(WaylandPlatformSurface *platformSurface)
{
    Q_D(SceneOpenGLTexture);
    d->update(platformSurface);
}

void SceneOpenGLTexture::updateTexture(X11PlatformSurface *platformSurface)
{
    Q_D(SceneOpenGLTexture);
    d->update(platformSurface);
}

SceneOpenGLTexture& SceneOpenGLTexture::operator = (const SceneOpenGLTexture& tex)
{
    d_ptr = tex.d_ptr;
    return *this;
}

void SceneOpenGLTexture::discard()
{
    d_ptr = d_func()->backend()->createBackendTexture(this);
}

SceneOpenGLTexturePrivate::SceneOpenGLTexturePrivate()
{
}

SceneOpenGLTexturePrivate::~SceneOpenGLTexturePrivate()
{
}

}
