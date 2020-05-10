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

#include "eglbuffer_wayland_p.h"
#include "abstract_egl_backend.h"
#include "egl_dmabuf.h"

#include <logging.h>
#include <kwinglplatform.h>
#include <kwingltexture.h>

#include <KWaylandServer/buffer_interface.h>
#include <KWaylandServer/surface_interface.h>

using namespace KWaylandServer;

namespace KWin
{

EGLBufferWaylandPrivate::EGLBufferWaylandPrivate(AbstractEglBackend *backend)
    : OpenGLBufferWaylandPrivate(backend)
{
}

EGLBufferWaylandPrivate::~EGLBufferWaylandPrivate()
{
    AbstractEglBackend *eglBackend = static_cast<AbstractEglBackend *>(backend);

    delete texture;

    if (textureId)
        glDeleteTextures(1, &textureId);
    if (m_image != EGL_NO_IMAGE_KHR)
        eglDestroyImage(eglBackend->eglDisplay(), m_image);
}

bool EGLBufferWaylandPrivate::loadDmaBufTexture()
{
    Q_ASSERT(m_image == EGL_NO_IMAGE_KHR);

    auto dmabuf = static_cast<EglDmabufBuffer *>(buffer->linuxDmabufBuffer());
    if (!dmabuf || dmabuf->images()[0] == EGL_NO_IMAGE_KHR) {
        qCritical(KWIN_OPENGL) << "Invalid dmabuf-based wl_buffer";
        delete texture;
        texture = nullptr;
        return false;
    }

    glGenTextures(1, &textureId);

    texture = new GLTexture(textureId, GL_RGBA, dmabuf->size());
    texture->setWrapMode(GL_CLAMP_TO_EDGE);
    texture->setFilter(GL_NEAREST);
    texture->bind();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) dmabuf->images()[0]);
    texture->unbind();
    texture->setYInverted(!(dmabuf->flags() & LinuxDmabufUnstableV1Interface::YInverted));

    return true;
}

bool EGLBufferWaylandPrivate::loadShmTexture()
{
    const QImage &image = buffer->data();
    if (image.isNull())
        return false;

    GLenum format = 0;
    switch (image.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        format = GL_RGBA8;
        break;
    case QImage::Format_RGB32:
        format = GL_RGB8;
        break;
    default:
        return false;
    }

    glGenTextures(1, &textureId);

    texture = new GLTexture(textureId, format, image.size());
    texture->setWrapMode(GL_CLAMP_TO_EDGE);
    texture->setFilter(GL_LINEAR);
    texture->bind();

    if (GLPlatform::instance()->isGLES()) {
        if (GLTexture::supportsARGB32() && format == GL_RGBA8) {
            const QImage im = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            glTexImage2D(texture->target(), 0, GL_BGRA_EXT, im.width(), im.height(),
                         0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, im.bits());
        } else {
            const QImage im = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
            glTexImage2D(texture->target(), 0, GL_RGBA, im.width(), im.height(),
                         0, GL_RGBA, GL_UNSIGNED_BYTE, im.bits());
        }
    } else {
        glTexImage2D(texture->target(), 0, format, texture->width(), texture->height(),
                     0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
    }

    texture->unbind();
    texture->setYInverted(true);

    return true;
}

bool EGLBufferWaylandPrivate::loadEglTexture()
{
//    if (!eglQueryWaylandBufferWL)
//        return false;
    if (!buffer->resource())
        return false;

    glGenTextures(1, &textureId);

    texture = new GLTexture(textureId, GL_RGBA, buffer->size());
    texture->setWrapMode(GL_CLAMP_TO_EDGE);
    texture->setFilter(GL_LINEAR);
    texture->bind();
    m_image = attach(buffer);
    texture->unbind();

    return m_image != EGL_NO_IMAGE_KHR;
}

bool EGLBufferWaylandPrivate::create()
{
    if (buffer->linuxDmabufBuffer()) {
        return loadDmaBufTexture();
    } else if (buffer->shmBuffer()) {
        return loadShmTexture();
    }
    return loadEglTexture();
}

void EGLBufferWaylandPrivate::updateDmaBufTexture()
{
    AbstractEglBackend *eglBackend = static_cast<AbstractEglBackend *>(backend);
    EglDmabufBuffer *dmabuf = static_cast<EglDmabufBuffer *>(buffer->linuxDmabufBuffer());

    if (m_image != EGL_NO_IMAGE_KHR) {
        eglDestroyImageKHR(eglBackend->eglDisplay(), m_image);
        m_image = EGL_NO_IMAGE_KHR;
    }

    texture->bind();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES) dmabuf->images()[0]);
    texture->unbind();

    // The origin in a dmabuf-buffer is at the upper-left corner, so the meaning
    // of Y-inverted is the inverse of OpenGL.
    const bool yInverted = !(dmabuf->flags() & LinuxDmabufUnstableV1Interface::YInverted);
    if (texture->isYInverted() != yInverted)
        texture->setYInverted(yInverted);
}

void EGLBufferWaylandPrivate::updateShmTexture()
{
    const QRegion damage = surface->trackedDamage();
    const qreal scale = surface->scale(); //damage is normalised, so needs converting up to match texture

    texture->bind();

    // TODO: this should be shared with GLTexture::update
    if (GLPlatform::instance()->isGLES()) {
        if (GLTexture::supportsARGB32() && (image.format() == QImage::Format_ARGB32 || image.format() == QImage::Format_ARGB32_Premultiplied)) {
            const QImage im = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            for (const QRect &rect : damage) {
                const QRect scaledRect(rect.x() * scale, rect.y() * scale,
                                       rect.width() * scale, rect.height() * scale);
                glTexSubImage2D(texture->target(), 0, scaledRect.x(), scaledRect.y(),
                                scaledRect.width(), scaledRect.height(),
                                GL_BGRA_EXT, GL_UNSIGNED_BYTE, im.copy(scaledRect).bits());
            }
        } else {
            const QImage im = image.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
            for (const QRect &rect : damage) {
                const QRect scaledRect(rect.x() * scale, rect.y() * scale,
                                       rect.width() * scale, rect.height() * scale);
                glTexSubImage2D(texture->target(), 0, scaledRect.x(), scaledRect.y(),
                                scaledRect.width(), scaledRect.height(),
                                GL_RGBA, GL_UNSIGNED_BYTE, im.copy(scaledRect).bits());
            }
        }
    } else {
        const QImage im = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        for (const QRect &rect : damage) {
            const QRect scaledRect(rect.x() * scale, rect.y() * scale,
                                   rect.width() * scale, rect.height() * scale);
            glTexSubImage2D(texture->target(), 0, scaledRect.x(), scaledRect.y(),
                            scaledRect.width(), scaledRect.height(),
                            GL_BGRA, GL_UNSIGNED_BYTE, im.copy(scaledRect).bits());
        }
    }
    texture->unbind();
}

void EGLBufferWaylandPrivate::updateEglTexture()
{
    AbstractEglBackend *eglBackend = static_cast<AbstractEglBackend *>(backend);

    texture->bind();
    EGLImageKHR image = attach(buffer);
    texture->unbind();

    if (image == EGL_NO_IMAGE_KHR)
        return;
    if (m_image != EGL_NO_IMAGE_KHR)
        eglDestroyImageKHR(eglBackend->eglDisplay(), m_image);

    m_image = image;
}

EGLImageKHR EGLBufferWaylandPrivate::attach(BufferInterface *buffer)
{
#if 0
    EGLint format, yInverted;
    eglQueryWaylandBufferWL(m_backend->eglDisplay(), buffer->resource(), EGL_TEXTURE_FORMAT, &format);
    if (format != EGL_TEXTURE_RGB && format != EGL_TEXTURE_RGBA) {
        qCDebug(KWIN_OPENGL) << "Unsupported texture format: " << format;
        return EGL_NO_IMAGE_KHR;
    }
    if (!eglQueryWaylandBufferWL(m_backend->eglDisplay(), buffer->resource(), EGL_WAYLAND_Y_INVERTED_WL, &yInverted)) {
        // if EGL_WAYLAND_Y_INVERTED_WL is not supported wl_buffer should be treated as if value were EGL_TRUE
        yInverted = EGL_TRUE;
    }

    const EGLint attribs[] = {
        EGL_WAYLAND_PLANE_WL, 0,
        EGL_NONE
    };
    EGLImageKHR image = eglCreateImageKHR(m_backend->eglDisplay(), EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL,
                                          (EGLClientBuffer)buffer->resource(), attribs);
    if (image != EGL_NO_IMAGE_KHR) {
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)image);
        q->setYInverted(yInverted);
    }
    return image;
#endif
    return EGL_NO_IMAGE;
}

void EGLBufferWaylandPrivate::update()
{
    if (buffer->linuxDmabufBuffer()) {
        updateDmaBufTexture();
    } else if (buffer->shmBuffer()) {
        updateShmTexture();
    } else {
        updateEglTexture();
    }
}

} // namespace KWin
