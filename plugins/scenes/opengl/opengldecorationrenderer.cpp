/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

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

#include "opengldecorationrenderer.h"
#include "abstract_client.h"
#include "composite.h"
#include "decorations/decoratedclient.h"
#include "scene.h"

#include "kwingltexture.h"

namespace KWin
{

OpenGLDecorationRenderer::OpenGLDecorationRenderer(Decoration::DecoratedClientImpl *client)
    : Renderer(client)
    , m_texture()
{
    connect(this, &Renderer::renderScheduled, client->client(), static_cast<void (AbstractClient::*)(const QRect&)>(&AbstractClient::addRepaint));
}

OpenGLDecorationRenderer::~OpenGLDecorationRenderer()
{
    if (Scene *scene = Compositor::self()->scene()) {
        scene->makeOpenGLContextCurrent();
    }
}

GLTexture *OpenGLDecorationRenderer::texture() const
{
    return m_texture.data();
}

// Rotates the given source rect 90° counter-clockwise,
// and flips it vertically
static QImage rotate(const QImage &srcImage, const QRect &srcRect)
{
    auto dpr = srcImage.devicePixelRatio();
    QImage image(srcRect.height() * dpr, srcRect.width() * dpr, srcImage.format());
    image.setDevicePixelRatio(dpr);
    const QPoint srcPoint(srcRect.x() * dpr, srcRect.y() * dpr);

    const uint32_t *src = reinterpret_cast<const uint32_t *>(srcImage.bits());
    uint32_t *dst = reinterpret_cast<uint32_t *>(image.bits());

    for (int x = 0; x < image.width(); x++) {
        const uint32_t *s = src + (srcPoint.y() + x) * srcImage.width() + srcPoint.x();
        uint32_t *d = dst + x;

        for (int y = 0; y < image.height(); y++) {
            *d = s[y];
            d += image.width();
        }
    }

    return image;
}

void OpenGLDecorationRenderer::render()
{
    const QRegion scheduled = getScheduled();
    const bool dirty = areImageSizesDirty();
    if (scheduled.isEmpty() && !dirty) {
        return;
    }
    if (dirty) {
        resizeTexture();
        resetImageSizesDirty();
    }

    if (!m_texture) {
        // for invalid sizes we get no texture, see BUG 361551
        return;
    }

    QRect left, top, right, bottom;
    client()->client()->layoutDecorationRects(left, top, right, bottom);

    const QRect geometry = dirty ? QRect(QPoint(0, 0), client()->client()->size()) : scheduled.boundingRect();

    auto renderPart = [this](const QRect &geo, const QRect &partRect, const QPoint &offset, bool rotated = false) {
        if (!geo.isValid()) {
            return;
        }
        QImage image = renderToImage(geo);
        if (rotated) {
            // TODO: get this done directly when rendering to the image
            image = rotate(image, QRect(geo.topLeft() - partRect.topLeft(), geo.size()));
        }
        m_texture->update(image, (geo.topLeft() - partRect.topLeft() + offset) * image.devicePixelRatio());
    };
    renderPart(left.intersected(geometry), left, QPoint(0, top.height() + bottom.height() + 2), true);
    renderPart(top.intersected(geometry), top, QPoint(0, 0));
    renderPart(right.intersected(geometry), right, QPoint(0, top.height() + bottom.height() + left.width() + 3), true);
    renderPart(bottom.intersected(geometry), bottom, QPoint(0, top.height() + 1));
}

static int align(int value, int align)
{
    return (value + align - 1) & ~(align - 1);
}

void OpenGLDecorationRenderer::resizeTexture()
{
    QRect left, top, right, bottom;
    client()->client()->layoutDecorationRects(left, top, right, bottom);
    QSize size;

    size.rwidth() = qMax(qMax(top.width(), bottom.width()),
                         qMax(left.height(), right.height()));
    size.rheight() = top.height() + bottom.height() +
                     left.width() + right.width() + 3;

    size.rwidth() = align(size.width(), 128);

    size *= client()->client()->screenScale();
    if (m_texture && m_texture->size() == size)
        return;

    if (!size.isEmpty()) {
        m_texture.reset(new GLTexture(GL_RGBA8, size.width(), size.height()));
        m_texture->setYInverted(true);
        m_texture->setWrapMode(GL_CLAMP_TO_EDGE);
        m_texture->clear();
    } else {
        m_texture.reset();
    }
}

void OpenGLDecorationRenderer::reparent(Deleted *deleted)
{
    render();
    Renderer::reparent(deleted);
}

} // namespace KWin
