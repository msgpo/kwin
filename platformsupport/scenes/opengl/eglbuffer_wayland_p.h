/*
 * Copyright (C) 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "openglbuffer_wayland_p.h"

#include <epoxy/egl.h>
#include <fixx11h.h>

namespace KWin
{

class AbstractEglBackend;

class KWIN_EXPORT EGLBufferWaylandPrivate : public OpenGLBufferWaylandPrivate
{
public:
    explicit EGLBufferWaylandPrivate(AbstractEglBackend *backend);
    ~EGLBufferWaylandPrivate() override;

    bool loadShmTexture();
    bool loadEglTexture();
    bool loadDmaBufTexture();

    void updateShmTexture();
    void updateEglTexture();
    void updateDmaBufTexture();

    EGLImageKHR attach(KWaylandServer::BufferInterface *buffer);

    bool create() override;
    void update() override;

    EGLImageKHR m_image = EGL_NO_IMAGE_KHR;
    GLuint textureId = 0;
};

} // namespace KWin
