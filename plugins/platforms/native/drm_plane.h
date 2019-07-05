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

#include "drm_object.h"

#include <QVector>

namespace KWin
{

class DrmCrtc;

/**
 * This enum type is used to specify plane type.
 */
enum PlaneType {
    PlanePrimary,
    PlaneOverlay,
    PlaneCursor,
};

struct PlaneProperties
{
    // All drivers should provide these properties.
    uint32_t crtcX = 0;
    uint32_t crtcY = 0;
    uint32_t crtcWidth = 0;
    uint32_t crtcHeight = 0;
    uint32_t crtcId = 0;
    uint32_t frameBufferId = 0;
    uint32_t srcX = 0;
    uint32_t srcY = 0;
    uint32_t srcWidth = 0;
    uint32_t srcHeight = 0;
};

class DrmPlane : public DrmObject
{
public:
    DrmPlane(DrmDevice *device, uint32_t objectId);
    ~DrmPlane() override;

    /**
     * Returns the type of this plane.
     */
    PlaneType type() const;

    /**
     * Returns CRTC that this plane is currently attached to.
     */
    DrmCrtc *crtc() const;

    /**
     * Assigns this plane to the given CRTC.
     */
    void setCrtc(DrmCrtc *crtc);

    /**
     * Returns supported formats.
     */
    QVector<uint32_t> formats() const;

    /**
     * Returns modifiers for the given format.
     */
    QVector<uint64_t> modifiers(uint32_t format) const;

private:
    PlaneType m_type = PlaneOverlay;
    PlaneProperties m_properties;

    DrmCrtc *m_crtc = nullptr;

    QVector<uint32_t> m_formats;
    QVector<drm_format_modifier> m_modifiers;

    Q_DISABLE_COPY(DrmPlane)
};

} // namespace KWin
