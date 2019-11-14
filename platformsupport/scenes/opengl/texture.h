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
#pragma once

#include <kwingltexture.h>
#include <kwingltexture_p.h>

namespace KWin
{

class InternalPlatformSurface;
class OpenGLBackend;
class SceneOpenGLTexturePrivate;
class WaylandPlatformSurface;
class X11PlatformSurface;

class SceneOpenGLTexture : public GLTexture
{
public:
    explicit SceneOpenGLTexture(OpenGLBackend *backend);
    ~SceneOpenGLTexture() override;

    bool loadTexture(InternalPlatformSurface *platformSurface);
    bool loadTexture(WaylandPlatformSurface *platformSurface);
    bool loadTexture(X11PlatformSurface *platformSurface);

    void updateTexture(InternalPlatformSurface *platformSurface);
    void updateTexture(WaylandPlatformSurface *platformSurface);
    void updateTexture(X11PlatformSurface *platformSurface);

    SceneOpenGLTexture & operator = (const SceneOpenGLTexture& tex);

    void discard() override final;

private:
    SceneOpenGLTexture(SceneOpenGLTexturePrivate& dd);

    Q_DECLARE_PRIVATE(SceneOpenGLTexture)
};

class SceneOpenGLTexturePrivate : public GLTexturePrivate
{
public:
    ~SceneOpenGLTexturePrivate() override;

    virtual bool create(InternalPlatformSurface *platformSurface) = 0;
    virtual bool create(WaylandPlatformSurface *platformSurface) = 0;
    virtual bool create(X11PlatformSurface *platformSurface) = 0;

    virtual void update(InternalPlatformSurface *platformSurface) = 0;
    virtual void update(WaylandPlatformSurface *platformSurface) = 0;
    virtual void update(X11PlatformSurface *platformSurface) = 0;

    virtual OpenGLBackend *backend() = 0;

protected:
    SceneOpenGLTexturePrivate();

private:
    Q_DISABLE_COPY(SceneOpenGLTexturePrivate)
};

}
