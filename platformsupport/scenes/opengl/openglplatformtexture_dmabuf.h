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

#include "openglplatformtexture_p.h"

#include <epoxy/egl.h>
#include <fixx11h.h>

namespace KWin
{

class OpenGLDmaBufPlatformTexturePrivate : public OpenGLPlatformTexturePrivate
{
public:
    explicit OpenGLDmaBufPlatformTexturePrivate(OpenGLPlatformTexture *texture);
    ~OpenGLDmaBufPlatformTexturePrivate() override;

    bool isCompatible(const WaylandPlatformSurface *platformSurface) const override final;
    bool create(WaylandPlatformSurface *platformSurface) override final;
    bool update(WaylandPlatformSurface *platformSurface, const QRegion &damage) override final;

private:
    EGLImageKHR m_image = EGL_NO_IMAGE_KHR;
};

} // namespace KWin
