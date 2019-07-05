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

#include "drm_gbm_image.h"
#include "drm_buffer.h"

namespace KWin
{

DrmGbmImage::DrmGbmImage(DrmDevice *device, gbm_bo *nativeHandle)
    : m_nativeHandle(nativeHandle)
{
    const uint32_t width = gbm_bo_get_width(m_nativeHandle);
    const uint32_t height = gbm_bo_get_height(m_nativeHandle);
    const uint32_t format = gbm_bo_get_format(m_nativeHandle);

    std::array<uint32_t, 4> handles = { 0 };
    std::array<uint32_t, 4> strides = { 0 };
    std::array<uint32_t, 4> offsets = { 0 };
    std::array<uint64_t, 4> modifiers = { 0 };

    const int planeCount = gbm_bo_get_plane_count(m_nativeHandle);
    for (int i = 0; i < planeCount; ++i) {
        handles[i] = gbm_bo_get_handle_for_plane(m_nativeHandle, i).u32;
        strides[i] = gbm_bo_get_stride_for_plane(m_nativeHandle, i);
        offsets[i] = gbm_bo_get_offset(m_nativeHandle, i);
        modifiers[i] = gbm_bo_get_modifier(m_nativeHandle);
    }

    m_buffer = DrmBuffer::create(device, width, height, format, planeCount,
        handles, strides, offsets, modifiers);
}

DrmGbmImage::~DrmGbmImage()
{
    delete m_buffer;

    gbm_bo_destroy(m_nativeHandle);
}

bool DrmGbmImage::isValid() const
{
    return m_buffer;
}

DrmBuffer *DrmGbmImage::buffer() const
{
    return m_buffer;
}

} // namespace KWin
