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

#include "drm_connector.h"
#include "drm_edid.h"

namespace KWin
{

DrmConnector::DrmConnector(DrmDevice *device, uint32_t objectId)
    : DrmObject(device, objectId, DRM_MODE_OBJECT_CONNECTOR)
{
    // forEachProperty([=](const drmModePropertyPtr property, uint64_t value) {
    //     if (property->name == QByteArrayLiteral("CRTC_ID")) {
    //         m_properties.crtcId = property->prop_id;
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("DPMS")) {
    //         m_properties.dpms = property->prop_id;
    //         return;
    //     }
    //     if (property->name == QByteArrayLiteral("EDID")) {
    //         return;
    //     }
    // });
}

DrmConnector::~DrmConnector()
{
}

bool DrmConnector::isOnline() const
{
    return m_isOnline;
}

DrmCrtc *DrmConnector::crtc() const
{
    return nullptr;
}

DrmEdid *DrmConnector::edid() const
{
    return nullptr;
}

} // namespace KWin
