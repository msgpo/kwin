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

namespace KWin
{

class DrmDevice;

/**
 * The DrmBlob class represents an owned property blob object.
 */
class DrmBlob
{
public:
    DrmBlob(DrmDevice *device, const void *data, size_t size);
    ~DrmBlob();

    /**
     * Returns @c true if the blob object is valid, otherwise @c false.
     */
    bool isValid() const;

    /**
     * Returns the id of the property blob object.
     */
    uint32_t objectId() const;

private:
    DrmDevice *m_device = nullptr;
    uint32_t m_id = 0;
    bool m_isValid = false;

    Q_DISABLE_COPY(DrmBlob)
};

} // namespace KWin
