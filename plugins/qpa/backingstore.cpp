/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>
Copyright (C) 2019 Vlad Zagorodniy <vladzzag@gmail.com>

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
#include "backingstore.h"
#include "window.h"

#include "internal_client.h"

#include <QPainter>

namespace KWin
{
namespace QPA
{

BackingStore::BackingStore(QWindow *window)
    : QPlatformBackingStore(window)
{
}

BackingStore::~BackingStore() = default;

QPaintDevice *BackingStore::paintDevice()
{
    return &m_backBuffer;
}

void BackingStore::resize(const QSize &size, const QRegion &staticContents)
{
    Q_UNUSED(staticContents)

    if (m_backBuffer.size() == size) {
        return;
    }

    const QPlatformWindow *platformWindow = static_cast<QPlatformWindow *>(window()->handle());
    const qreal devicePixelRatio = platformWindow->devicePixelRatio();

    m_backBuffer = QImage(size * devicePixelRatio, QImage::Format_ARGB32_Premultiplied);
    m_backBuffer.setDevicePixelRatio(devicePixelRatio);

    m_frontBuffer = QImage(size * devicePixelRatio, QImage::Format_ARGB32_Premultiplied);
    m_frontBuffer.setDevicePixelRatio(devicePixelRatio);
}

static void blitImage(const QImage &source, QImage &target, const QRect &rect)
{
    Q_ASSERT(source.format() == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(target.format() == QImage::Format_ARGB32_Premultiplied);

    const int devicePixelRatio = target.devicePixelRatio();

    const int x = rect.x() * devicePixelRatio;
    const int y = rect.y() * devicePixelRatio;
    const int width = rect.width() * devicePixelRatio;
    const int height = rect.height() * devicePixelRatio;

    for (int i = y; i < y + height; ++i) {
        const uint32_t *in = reinterpret_cast<const uint32_t *>(source.scanLine(i));
        uint32_t *out = reinterpret_cast<uint32_t *>(target.scanLine(i));
        std::copy(in + x, in + x + width, out + x);
    }
}

static void blitImage(const QImage &source, QImage &target, const QRegion &region)
{
    for (const QRect &rect : region) {
        blitImage(source, target, rect);
    }
}

void BackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(offset)

    Window *platformWindow = static_cast<Window *>(window->handle());
    InternalClient *client = platformWindow->client();
    if (!client) {
        return;
    }

    blitImage(m_backBuffer, m_frontBuffer, region);

    client->present(m_frontBuffer, region);
}

}
}
