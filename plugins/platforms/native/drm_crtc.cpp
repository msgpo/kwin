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

#include "drm_crtc.h"

namespace KWin
{

DrmCrtc::DrmCrtc(DrmDevice *device, uint32_t objectId, uint32_t pipe)
    : DrmObject(device, objectId, DRM_MODE_OBJECT_CRTC)
    , m_pipe(pipe)
{
    // forEachProperty([this](const drmModePropertyPtr property, uint64_t value) {
    //     Q_UNUSED(value)
    //     if (property->name == QByteArrayLiteral("ACTIVE")) {
    //         m_properties.active = property->prop_id;
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("DEGAMMA_LUT")) {
    //         m_properties.degammaTable = property->prop_id;
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("DEGAMMA_LUT_SIZE")) {
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("GAMMA_LUT")) {
    //         m_properties.gammaTable = property->prop_id;
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("GAMMA_LUT_SIZE")) {
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("MODE_ID")) {
    //         m_properties.modeId = property->prop_id;
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("rotation")) {
    //         m_properties.rotation = property->prop_id;
    //         return;
    //     }
    // });
}

DrmCrtc::~DrmCrtc()
{
}

uint32_t DrmCrtc::pipe() const
{
    return m_pipe;
}

DrmPlane *DrmCrtc::primaryPlane() const
{
    return m_primaryPlane;
}

void DrmCrtc::setPrimaryPlane(DrmPlane *plane)
{
    m_primaryPlane = plane;
}

DrmPlane *DrmCrtc::cursorPlane() const
{
    return m_cursorPlane;
}

void DrmCrtc::setCursorPlane(DrmPlane *plane)
{
    m_cursorPlane = plane;
}

} // namespace KWin
