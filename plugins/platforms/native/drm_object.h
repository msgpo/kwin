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

#include <functional>

#include <xf86drmMode.h>

namespace KWin
{

class DrmDevice;

/**
 * The DrmObject class is a base class for DRM objects.
 */
class DrmObject
{
public:
    DrmObject(DrmDevice *device, uint32_t objectId, uint32_t objectType);
    virtual ~DrmObject();

    /**
     * Returns a device this object belongs to.
     */
    DrmDevice *device() const;

    /**
     * Returns the id of this DRM object.
     */
    uint32_t objectId() const;

    /**
     * Returns the type of this DRM object.
     */
    uint32_t objectType() const;

protected:
    /**
     * Enumerates all properties that this object has.
     */
    void forEachProperty(
        std::function<void(const drmModePropertyPtr, uint64_t)> callback);

private:
    DrmDevice *m_device;
    uint32_t m_objectId;
    uint32_t m_objectType;

    Q_DISABLE_COPY(DrmObject)
};

} // namespace KWin
