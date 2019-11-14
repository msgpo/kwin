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

#pragma once

#include "kwingltexture_p.h"

namespace KWin
{

class InternalPlatformSurface;
class OpenGLBackend;
class OpenGLPlatformTexture;
class WaylandPlatformSurface;
class X11PlatformSurface;

class OpenGLPlatformTexturePrivate : public GLTexturePrivate
{
public:
    explicit OpenGLPlatformTexturePrivate(OpenGLPlatformTexture *texture);
    ~OpenGLPlatformTexturePrivate() override;

    OpenGLBackend *backend() const;

    virtual bool isCompatible(const InternalPlatformSurface *platformSurface) const;
    virtual bool isCompatible(const WaylandPlatformSurface *platformSurface) const;
    virtual bool isCompatible(const X11PlatformSurface *platformSurface) const;

    virtual bool create(InternalPlatformSurface *platformsurface);
    virtual bool create(WaylandPlatformSurface *platformSurface);
    virtual bool create(X11PlatformSurface *platformSurface);

    virtual bool update(InternalPlatformSurface *platformSurface, const QRegion &damage);
    virtual bool update(WaylandPlatformSurface *platformSurface, const QRegion &damage);
    virtual bool update(X11PlatformSurface *platformSurface, const QRegion &damage);

protected:
    OpenGLPlatformTexture *q;
};

} // namespace KWin
