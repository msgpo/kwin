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

namespace KWin
{

class OpenGLImagePlatformTexturePrivate final : public OpenGLPlatformTexturePrivate
{
public:
    explicit OpenGLImagePlatformTexturePrivate(OpenGLPlatformTexture *texture);
    ~OpenGLImagePlatformTexturePrivate() override;

    using OpenGLPlatformTexturePrivate::isCompatible;
    using OpenGLPlatformTexturePrivate::create;
    using OpenGLPlatformTexturePrivate::update;

    bool isCompatible(const InternalPlatformSurface *platformSurface) const override;
    bool create(InternalPlatformSurface *platformSurface) override;
    bool update(InternalPlatformSurface *platformSurface, const QRegion &damage) override;
};

} // namespace KWin
