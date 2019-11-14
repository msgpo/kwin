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
#include "egl_dmabuf.h"
#include "openglplatformtexture.h"

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

static EglDmabufBuffer *dmabufFromPlatformSurface(WaylandPlatformSurface *platformSurface)
{
    KWayland::Server::BufferInterface *buffer = platformSurface->buffer();
    return static_cast<EglDmabufBuffer *>(buffer->linuxDmabufBuffer());
}

bool OpenGLDmaBufPlatformTexturePrivate::create(WaylandPlatformSurface *platformSurface)
{
    EglDmabufBuffer *dmabuf = dmabufFromPlatformSurface(platformSurface);
    if (!dmabuf) {
        return false;
    }

    glGenTextures(1, &m_texture);
    q->setWrapMode(GL_CLAMP_TO_EDGE);
    q->setFilter(GL_NEAREST);

    q->bind();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, dmabuf->images().first());
    q->unbind();

    m_size = dmabuf->size();
    q->setYInverted(!(dmabuf->flags() & KWayland::Server::LinuxDmabufUnstableV1Interface::YInverted));

    return false;
}

bool OpenGLDmaBufPlatformTexturePrivate::update(WaylandPlatformSurface *platformSurface, const QRegion &damage)
{
    Q_UNUSED(damage)

    EglDmabufBuffer *dmabuf = dmabufFromPlatformSurface(platformSurface);
    if (!dmabuf) {
        return false;
    }

    q->bind();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, dmabuf->images().first());
    q->unbind();

    m_size = dmabuf->size();
    q->setYInverted(!(dmabuf->flags() & KWayland::Server::LinuxDmabufUnstableV1Interface::YInverted));

    return false;
}

} // namespace KWin
