/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>
Copyright (C) 2009, 2010, 2011 Martin Gräßlin <mgraesslin@kde.org>
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

#include "openglwindowpixmap.h"
#include "scene_opengl.h"

#include "platformsupport/scenes/opengl/texture.h"

#include <KWayland/Server/surface_interface.h>

namespace KWin
{

OpenGLWindowPixmap::OpenGLWindowPixmap(Scene::Window *window, SceneOpenGL *scene)
    : WindowPixmap(window)
    , m_texture(scene->createTexture())
    , m_scene(scene)
{
}

OpenGLWindowPixmap::~OpenGLWindowPixmap()
{
}

SceneOpenGLTexture *OpenGLWindowPixmap::texture() const
{
    return m_texture.data();
}

static bool needsPixmapUpdate(const OpenGLWindowPixmap *pixmap)
{
    // That's a regular Wayland client.
    if (pixmap->surface()) {
        return !pixmap->surface()->trackedDamage().isEmpty();
    }

    // That's an internal client with a raster buffer attached.
    if (!pixmap->internalImage().isNull()) {
        return !pixmap->toplevel()->damage().isEmpty();
    }

    // That's an internal client with an opengl framebuffer object attached.
    if (!pixmap->fbo().isNull()) {
        return !pixmap->toplevel()->damage().isEmpty();
    }

    // That's an X11 client.
    return false;
}

bool OpenGLWindowPixmap::bind()
{
    if (!m_texture->isNull()) {
        if (needsPixmapUpdate(this)) {
            m_texture->updateFromPixmap(this);
            // mipmaps need to be updated
            m_texture->setDirty();
        }
        return true;
    }
    if (!isValid()) {
        return false;
    }

    bool success = m_texture->load(this);

    return success;
}

bool OpenGLWindowPixmap::isValid() const
{
    if (!m_texture->isNull()) {
        return true;
    }
    return WindowPixmap::isValid();
}

} // namespace KWin
