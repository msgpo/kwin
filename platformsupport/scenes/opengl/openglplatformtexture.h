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

#include "kwingltexture.h"

namespace KWin
{

class InternalPlatformSurface;
class OpenGLBackend;
class WaylandPlatformSurface;
class X11PlatformSurface;

class OpenGLPlatformTexture : public GLTexture
{
public:
    explicit OpenGLPlatformTexture(OpenGLBackend *backend);
    ~OpenGLPlatformTexture() override;

    OpenGLBackend *backend() const;

    bool create(InternalPlatformSurface *platformSurface);
    bool create(WaylandPlatformSurface *platformSurface);
    bool create(X11PlatformSurface *platformSurface);

    bool update(InternalPlatformSurface *platformSurface);
    bool update(WaylandPlatformSurface *platformSurface);
    bool update(X11PlatformSurface *platformSurface);

    using GLTexture::update;

private:
    OpenGLBackend *m_backend;
};

} // namespace KWin
