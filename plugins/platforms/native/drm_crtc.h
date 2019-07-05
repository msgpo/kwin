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

namespace KWin
{

class DrmPlane;

class DrmCrtc : public DrmObject
{
public:
    DrmCrtc(DrmDevice *device, uint32_t objectId, uint32_t pipe);
    ~DrmCrtc() override;

    /**
     * Returns index of this CRTC in drmModeRes array.
     */
    uint32_t pipe() const;

    /**
     * Returns the primary plane.
     */
    DrmPlane *primaryPlane() const;

    /**
     * Assigns the primary plane to this CRTC.
     */
    void setPrimaryPlane(DrmPlane *plane);

    /**
     * Returns the cursor plane.
     *
     * If there is no such a plane, @c null is returned.
     */
    DrmPlane *cursorPlane() const;

    /**
     * Assigns the cursor plane to this CRTC.
     */
    void setCursorPlane(DrmPlane *plane);

private:
    DrmPlane *m_primaryPlane = nullptr;
    DrmPlane *m_cursorPlane = nullptr;

    uint32_t m_pipe;

    Q_DISABLE_COPY(DrmCrtc)
};

} // namespace KWin
