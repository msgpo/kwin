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

#include "drm_buffer.h"
#include "drm_device.h"

#include <drm_fourcc.h>
#include <xf86drmMode.h>

namespace KWin
{

DrmBuffer::DrmBuffer()
{
}

DrmBuffer::~DrmBuffer()
{
    if (m_id) {
        drmModeRmFB(m_device->fd(), m_id);
    }
}

DrmDevice *DrmBuffer::device() const
{
    return m_device;
}

uint32_t DrmBuffer::id() const
{
    return m_id;
}

uint32_t DrmBuffer::format() const
{
    return m_format;
}

uint32_t DrmBuffer::width() const
{
    return m_width;
}

uint32_t DrmBuffer::height() const
{
    return m_height;
}

uint32_t DrmBuffer::planeCount() const
{
    return m_planeCount;
}

uint32_t DrmBuffer::handle(int plane) const
{
    return m_handles[plane];
}

uint32_t DrmBuffer::pitch(int plane) const
{
    return m_pitches[plane];
}

uint64_t DrmBuffer::modifier() const
{
    return m_modifier;
}

DrmBuffer *DrmBuffer::create(DrmDevice *device,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint32_t planeCount,
    const std::array<uint32_t, 4> &handles,
    const std::array<uint32_t, 4> &pitches,
    const std::array<uint32_t, 4> &offsets,
    const std::array<uint64_t, 4> &modifiers)
{
    uint32_t id;
    bool ok;

    if (modifiers[0] != DRM_FORMAT_MOD_INVALID) {
        ok = drmModeAddFB2WithModifiers(device->fd(), width, height, format,
            handles.data(), pitches.data(), offsets.data(), modifiers.data(),
            &id, DRM_MODE_FB_MODIFIERS);
    } else {
        ok = drmModeAddFB2(device->fd(), width, height, format, handles.data(),
            pitches.data(), offsets.data(), &id, 0);
    }

    if (!ok) {
        return nullptr;
    }

    DrmBuffer *buffer = new DrmBuffer();
    buffer->m_handles = handles;
    buffer->m_pitches = pitches;
    buffer->m_offsets = offsets;
    buffer->m_device = device;
    buffer->m_modifier = modifiers[0];
    buffer->m_id = id;
    buffer->m_format = format;
    buffer->m_width = width;
    buffer->m_height = height;
    buffer->m_planeCount = planeCount;

    return buffer;
}

} // namespace KWin
