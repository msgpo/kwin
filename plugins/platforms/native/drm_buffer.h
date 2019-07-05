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

#include <QtGlobal>

#include <array>

namespace KWin
{

class DrmDevice;

/**
 * The DrmBuffer class ...
 */
class DrmBuffer
{
public:
    ~DrmBuffer();

    /**
     * Returns device to which this frame buffer belongs to.
     */
    DrmDevice *device() const;

    /**
     * Returns the id of the frame buffer.
     */
    uint32_t id() const;

    /**
     * Returns format of the frame buffer.
     */
    uint32_t format() const;

    /**
     * Returns the width of the frame buffer.
     */
    uint32_t width() const;

    /**
     * Returns the height of the frame buffer.
     */
    uint32_t height() const;

    /**
     * Returns the number of planes.
     */
    uint32_t planeCount() const;

    /**
     * Returns the handle of the given plane.
     */
    uint32_t handle(int plane) const;

    /**
     * Returns the pitch of the given plane.
     */
    uint32_t pitch(int plane) const;

    /**
     * Returns the offset of the given plane.
     */
    uint32_t offset(int plane) const;

    /**
     *
     */
    uint64_t modifier() const;

    /**
     * Creates a DRM framebuffer object.
     *
     * @param device
     * @param width The width of the frame buffer.
     * @param height The height of the frame buffer.
     * @param format The pixel format of the frame buffer.
     * @param planeCount The number of color planes.
     * @param handles
     * @param pitches
     * @param offsets
     * @param modifiers
     */
    static DrmBuffer *create(DrmDevice *device,
        uint32_t width,
        uint32_t height,
        uint32_t format,
        uint32_t planeCount,
        const std::array<uint32_t, 4> &handles,
        const std::array<uint32_t, 4> &pitches,
        const std::array<uint32_t, 4> &offsets,
        const std::array<uint64_t, 4> &modifiers);

private:
    explicit DrmBuffer();

    std::array<uint32_t, 4> m_handles;
    std::array<uint32_t, 4> m_pitches;
    std::array<uint32_t, 4> m_offsets;

    DrmDevice *m_device;

    uint64_t m_modifier;

    uint32_t m_id;
    uint32_t m_format;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_planeCount;

    Q_DISABLE_COPY(DrmBuffer)
};

} // namespace KWin
