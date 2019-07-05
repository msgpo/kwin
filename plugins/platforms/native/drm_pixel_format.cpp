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

#include "drm_pixel_format.h"

#include <drm_fourcc.h>

namespace KWin
{

DrmPixelFormat::DrmPixelFormat(uint32_t format)
{
    // TODO: Finish it.

    switch (format) {
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XBGR8888:
        m_bitsPerPixel = 32;
        break;

    default:
        m_bitsPerPixel = 0;
        break;
    }

    switch (format) {
    case DRM_FORMAT_ARGB8888:
    case DRM_FORMAT_XRGB8888:
    case DRM_FORMAT_ABGR8888:
    case DRM_FORMAT_XBGR8888:
        m_planeCount = 1;
        break;

    default:
        m_planeCount = 1;
    }
}

int DrmPixelFormat::bitsPerPixel() const
{
    return m_bitsPerPixel;
}

int DrmPixelFormat::planeCount() const
{
    return m_planeCount;
}

} // namespace KWin
