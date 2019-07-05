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

#include "drm_plane.h"
#include "drm_device.h"
#include "drm_pointer.h"

namespace KWin
{

static PlaneType realizePlaneType(const drmModePropertyPtr property, uint64_t value)
{
    for (int i = 0; i < property->count_enums; ++i) {
        const drm_mode_property_enum &enumerator = property->enums[i];
        if (enumerator.value != value) {
            continue;
        }
        if (enumerator.name == QByteArrayLiteral("Primary")) {
            return PlanePrimary;
        }
        if (enumerator.name == QByteArrayLiteral("Cursor")) {
            return PlaneCursor;
        }
        if (enumerator.name == QByteArrayLiteral("Overlay")) {
            return PlaneOverlay;
        }
    }

    Q_UNREACHABLE();
}

static QVector<uint32_t> parseFormats(const drmModePlane *plane)
{
    QVector<uint32_t> formats;
    formats.reserve(plane->count_formats);

    for (uint32_t i = 0; i < plane->count_formats; ++i) {
        formats.append(plane->formats[i]);
    }

    return formats;
}

static QVector<uint32_t> parseFormats(const uint8_t *data)
{
    auto header = reinterpret_cast<const drm_format_modifier_blob *>(data);
    auto formats = reinterpret_cast<const uint32_t *>(data + header->formats_offset);

    QVector<uint32_t> supportedFormats;
    supportedFormats.reserve(header->count_formats);

    for (uint32_t i = 0; i < header->count_formats; ++i) {
        supportedFormats.append(formats[i]);
    }

    return supportedFormats;
}

static QVector<drm_format_modifier> parseModifiers(const uint8_t *data)
{
    auto header = reinterpret_cast<const drm_format_modifier_blob *>(data);
    auto modifiers = reinterpret_cast<const drm_format_modifier *>(data + header->modifiers_offset);

    QVector<drm_format_modifier> supportedModifiers;
    supportedModifiers.reserve(header->count_modifiers);

    for (uint32_t i = 0; i < header->count_modifiers; ++i) {
        supportedModifiers.append(modifiers[i]);
    }

    return supportedModifiers;
}

DrmPlane::DrmPlane(DrmDevice *device, uint32_t objectId)
    : DrmObject(device, objectId, DRM_MODE_OBJECT_PLANE)
{
    const int fd = device->fd();

    DrmScopedPointer<drmModePlane> plane(drmModeGetPlane(fd, objectId));
    if (!plane) {
        return;
    }

    forEachProperty([=](const drmModePropertyPtr property, uint64_t value) {
        if (property->name == QByteArrayLiteral("CRTC_X")) {
            m_properties.crtcX = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("CRTC_Y")) {
            m_properties.crtcY = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("CRTC_W")) {
            m_properties.crtcWidth = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("CRTC_H")) {
            m_properties.crtcHeight = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("CRTC_ID")) {
            m_properties.crtcId = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("FB_ID")) {
            m_properties.frameBufferId = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("IN_FORMATS")) {
            const uint32_t id = uint32_t(value);
            DrmScopedPointer<drmModePropertyBlobRes> blob(drmModeGetPropertyBlob(fd, id));
            if (!blob) {
                return;
            }
            m_formats = parseFormats(static_cast<uint8_t *>(blob->data));
            m_modifiers = parseModifiers(static_cast<uint8_t *>(blob->data));
            return;
        }
        if (property->name == QByteArrayLiteral("SRC_X")) {
            m_properties.srcX = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("SRC_Y")) {
            m_properties.srcY = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("SRC_W")) {
            m_properties.srcWidth = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("SRC_H")) {
            m_properties.srcHeight = property->prop_id;
            return;
        }
        if (property->name == QByteArrayLiteral("type")) {
            m_type = realizePlaneType(property, value);
            return;
        }
    });

    m_crtc = device->findCrtc(plane->crtc_id);

    if (m_formats.isEmpty()) {
        m_formats = parseFormats(plane.get());
    }
}

DrmPlane::~DrmPlane()
{
}

PlaneType DrmPlane::type() const
{
    return m_type;
}

QVector<uint32_t> DrmPlane::formats() const
{
    return m_formats;
}

QVector<uint64_t> DrmPlane::modifiers(uint32_t format) const
{
    QVector<uint64_t> modifiers;

    const int formatIndex = m_formats.indexOf(format);
    if (formatIndex == -1) {
        return modifiers;
    }

    for (const drm_format_modifier& modifier : m_modifiers) {
        if (modifier.formats & (1ull << (formatIndex - modifier.offset))) {
            modifiers << modifier.modifier;
        }
    }

    return modifiers;
}

DrmCrtc *DrmPlane::crtc() const
{
    return m_crtc;
}

void DrmPlane::setCrtc(DrmCrtc *crtc)
{
    m_crtc = crtc;
}

} // namespace KWin
