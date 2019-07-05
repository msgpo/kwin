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

#include "drm_dumb_image.h"
#include "drm_device.h"
#include "drm_buffer.h"
#include "drm_pixel_format.h"

#include <drm_fourcc.h>
#include <xf86drm.h>

namespace KWin
{

DrmDumbImage::DrmDumbImage(DrmDevice *device, uint32_t width, uint32_t height, uint32_t format)
    : m_device(device)
{
    const DrmPixelFormat pixelInfo(format);
    if (pixelInfo.planeCount() != 1) {
        return;
    }

    drm_mode_create_dumb createInfo = {};
    createInfo.bpp = pixelInfo.bitsPerPixel();
    createInfo.width = width;
    createInfo.height = height;

    if (drmIoctl(device->fd(), DRM_IOCTL_MODE_CREATE_DUMB, &createInfo)) {
        return;
    }

    const std::array<uint32_t, 4> handles = { createInfo.handle };
    const std::array<uint32_t, 4> pitches = { createInfo.pitch };
    const std::array<uint32_t, 4> offsets = { 0 };
    const std::array<uint64_t, 4> modifiers = { DRM_FORMAT_MOD_INVALID };

    const int planeCount = 1;

    m_buffer = DrmBuffer::create(device, width, height, format, planeCount,
        handles, pitches, offsets, modifiers);

    m_handle = createInfo.handle;
}

DrmDumbImage::~DrmDumbImage()
{
    delete m_buffer;

    if (m_handle) {
        drm_mode_destroy_dumb destroyInfo = {};
        destroyInfo.handle = m_handle;
        drmIoctl(m_device->fd(), DRM_IOCTL_MODE_DESTROY_DUMB, &destroyInfo);
    }
}

bool DrmDumbImage::isValid() const
{
    return m_buffer;
}

DrmBuffer *DrmDumbImage::buffer() const
{
    return m_buffer;
}

} // namespace KWin
