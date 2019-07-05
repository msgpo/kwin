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

#include "drm_gbm_allocator.h"
#include "drm_device.h"
#include "drm_gbm_image.h"

namespace KWin
{

DrmGbmAllocator::DrmGbmAllocator(DrmDevice *device, QObject *parent)
    : DrmAllocator(device, parent)
    , m_device(gbm_create_device(device->fd()))
{
}

DrmGbmAllocator::~DrmGbmAllocator()
{
    if (m_device) {
        gbm_device_destroy(m_device);
    }
}

bool DrmGbmAllocator::isValid() const
{
    return m_device;
}

DrmImage *DrmGbmAllocator::allocate(uint32_t width,
    uint32_t height,
    uint32_t format,
    const QVector<uint64_t> &modifiers)
{
    // TODO: Properly handle linear modifier.
    // TODO: Maybe have usage flag as well?

    gbm_bo *bo;

    if (!modifiers.isEmpty()) {
        bo = gbm_bo_create_with_modifiers(m_device, width, height, format,
            modifiers.data(), modifiers.count());
    } else {
        const uint32_t flags = GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT;
        bo = gbm_bo_create(m_device, width, height, format, flags);
    }

    if (!bo) {
        return nullptr;
    }

    return new DrmGbmImage(device(), bo);
}

gbm_device *DrmGbmAllocator::nativeHandle() const
{
    return m_device;
}

} // namespace KWin
