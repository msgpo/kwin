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

#include "openglplatformtexture_fbo.h"
#include "openglplatformtexture.h"

#include <QOpenGLFramebufferObject>

namespace KWin
{

OpenGLFramebufferObjectPlatformTexturePrivate::OpenGLFramebufferObjectPlatformTexturePrivate(OpenGLPlatformTexture *texture)
    : OpenGLPlatformTexturePrivate(texture)
{
}

OpenGLFramebufferObjectPlatformTexturePrivate::~OpenGLFramebufferObjectPlatformTexturePrivate()
{
}

bool OpenGLFramebufferObjectPlatformTexturePrivate::isCompatible(const InternalPlatformSurface *platformSurface) const
{
    return platformSurface->framebufferObject();
}

bool OpenGLFramebufferObjectPlatformTexturePrivate::create(InternalPlatformSurface *platformSurface)
{
    return importFramebufferObject(platformSurface->framebufferObject());
}

bool OpenGLFramebufferObjectPlatformTexturePrivate::update(InternalPlatformSurface *platformSurface, const QRegion &damage)
{
    Q_UNUSED(damage)
    return importFramebufferObject(platformSurface->framebufferObject());
}

bool OpenGLFramebufferObjectPlatformTexturePrivate::importFramebufferObject(const QSharedPointer<QOpenGLFramebufferObject> &fbo)
{
    if (!fbo) {
        return false;
    }

    m_texture = fbo->texture();
    m_size = fbo->size();

    q->setWrapMode(GL_CLAMP_TO_EDGE);
    q->setFilter(GL_LINEAR);
    q->setYInverted(false);
    updateMatrix();

    return true;
}

} // namespace KWin
