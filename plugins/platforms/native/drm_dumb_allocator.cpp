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

#include "drm_dumb_allocator.h"
#include "drm_device.h"
#include "drm_dumb_image.h"

namespace KWin
{

DrmDumbAllocator::DrmDumbAllocator(DrmDevice *device, QObject *parent)
    : DrmAllocator(device, parent)
{
}

DrmDumbAllocator::~DrmDumbAllocator()
{
}

bool DrmDumbAllocator::isValid() const
{
    return device()->supports(DrmDevice::DeviceCapabilityDumbBuffer);
}

DrmImage *DrmDumbAllocator::allocate(uint32_t width,
    uint32_t height,
    uint32_t format,
    const QVector<uint64_t> &modifiers)
{
    Q_UNUSED(modifiers)

    return new DrmDumbImage(device(), width, height, format);
}

} // namespace KWin
