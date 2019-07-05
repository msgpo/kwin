/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

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

#pragma once

#include <QObject>
#include <QVector>

namespace KWin
{

class DrmDevice;
class DrmImage;
class DrmImageView;

/**
 * The DrmFramebuffer class ...
 *
 * @todo Pick a better name. "Framebuffer" part can be confusing a bit.
 */
class DrmFramebuffer
{
public:
    DrmFramebuffer(DrmImage *image, DrmImageView *view);
    ~DrmFramebuffer();

    /**
     *
     */
    DrmImage *image() const;

    /**
     *
     */
    DrmImageView *view() const;

    /**
     *
     */
    uint sequence() const;

    /**
     *
     */
    void setSequence(uint sequence);

    /**
     * Returns @c true if this framebuffer object is already acquired.
     */
    bool isAcquired() const;

    /**
     * Acquires the framebuffer object.
     */
    void acquire();

    /**
     * Releases the framebuffer object.
     */
    void release();

private:
    DrmImage *m_image = nullptr;
    DrmImageView *m_view = nullptr;
    uint m_sequence = 0;
    bool m_isAcquired = false;
};

/**
 * The DrmSwapchain class ...
 */
class DrmSwapchain : public QObject
{
    Q_OBJECT

public:
    explicit DrmSwapchain(DrmDevice *device, QObject *parent = nullptr);
    ~DrmSwapchain() override;

    /**
     * Returns @c true if the swapchain is valid, otherwise @c false.
     */
    bool isValid() const;

    /**
     * Returns the number of images in the swapchain.
     */
    int imageCount() const;

    /**
     * Acquires the next available image from the swapchain.
     *
     * If there are no available images, @c null will be returned.
     */
    DrmFramebuffer *acquire();

private:
    QVector<DrmFramebuffer *> m_framebuffers;

    Q_DISABLE_COPY(DrmSwapchain)
};

} // namespace KWin
